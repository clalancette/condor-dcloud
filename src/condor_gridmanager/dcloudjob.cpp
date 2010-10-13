/***************************************************************
 *
 * Copyright (C) 1990-2007, Condor Team, Computer Sciences Department,
 * University of Wisconsin-Madison, WI.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License"); you
 * may not use this file except in compliance with the License.  You may
 * obtain a copy of the License at
 * 
 *    http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************/

  
#include "condor_common.h"
#include "condor_attributes.h"
#include "condor_debug.h"
#include "condor_string.h"	// for strnewp and friends
#include "../condor_daemon_core.V6/condor_daemon_core.h"
#include "basename.h"
#include "nullfile.h"
#include "filename_tools.h"

#include "gridmanager.h"
#include "dcloudjob.h"
#include "condor_config.h"
  
#define GM_INIT							0
#define GM_UNSUBMITTED					1
#define GM_CREATE_VM					2
#define GM_SAVE_INSTANCE_ID				3
#define GM_SUBMITTED					4
#define GM_DONE_SAVE					5
#define GM_CANCEL						6
#define GM_STOPPED						7
#define GM_DELETE						8
#define GM_CLEAR_REQUEST				9
#define GM_HOLD							10
#define GM_PROBE_JOB					11
#define GM_START						12
#define GM_SAVE_INSTANCE_NAME			13
#define GM_CHECK_VM						14
#define GM_START_VM						15

static const char *GMStateNames[] = {
	"GM_INIT",
	"GM_UNSUBMITTED",
	"GM_CREATE_VM",
	"GM_SAVE_INSTANCE_ID",
	"GM_SUBMITTED",
	"GM_DONE_SAVE",
	"GM_CANCEL",
	"GM_STOPPED",
	"GM_DELETE",
	"GM_CLEAR_REQUEST",
	"GM_HOLD",
	"GM_PROBE_JOB",
	"GM_START",
	"GM_SAVE_INSTANCE_NAME",
	"GM_CHECK_VM",
	"GM_START_VM",
};

#define DCLOUD_VM_STATE_RUNNING			"RUNNING"
#define DCLOUD_VM_STATE_PENDING			"PENDING"
#define DCLOUD_VM_STATE_SHUTTINGDOWN	"SHUTTING-DOWN"
#define DCLOUD_VM_STATE_STOPPED			"STOPPED"
#define DCLOUD_VM_STATE_FINISH			"FINISH"

// Filenames are case insensitive on Win32, but case sensitive on Unix
#ifdef WIN32
#	define file_strcmp _stricmp
#	define file_contains contains_anycase
#else
#	define file_strcmp strcmp
#	define file_contains contains
#endif

// TODO: Let the maximum submit attempts be set in the job ad or, better yet,
// evalute PeriodicHold expression in job ad.
#define MAX_SUBMIT_ATTEMPTS	1

HashTable<HashKey, DCloudJob *> DCloudJob::JobsByInstanceId( hashFunction );

void DCloudJobInit()
{
}


void DCloudJobReconfig()
{
	// change interval time for 5 minute
	int tmp_int = param_integer( "GRIDMANAGER_JOB_PROBE_INTERVAL", 60 * 5 ); 
	DCloudJob::setProbeInterval( tmp_int );
		
	// Tell all the resource objects to deal with their new config values
	DCloudResource *next_resource;

	DCloudResource::ResourcesByName.startIterations();

	while ( DCloudResource::ResourcesByName.iterate( next_resource ) != 0 ) {
		next_resource->Reconfig();
	}	
}


bool DCloudJobAdMatch( const ClassAd *job_ad )
{
	int universe;
	MyString resource;

	job_ad->LookupInteger( ATTR_JOB_UNIVERSE, universe );
	job_ad->LookupString( ATTR_GRID_RESOURCE, resource );

	if ( (universe == CONDOR_UNIVERSE_GRID) && (strncasecmp( resource.Value(), "dcloud", 6 ) == 0) ) 
	{
		return true;
	}
	return false;
}


BaseJob* DCloudJobCreate( ClassAd *jobad )
{
	return (BaseJob *)new DCloudJob( jobad );
}

int DCloudJob::gahpCallTimeout = 600;
int DCloudJob::probeInterval = 300;
int DCloudJob::submitInterval = 300;
int DCloudJob::maxConnectFailures = 3;
int DCloudJob::funcRetryInterval = 15;
int DCloudJob::pendingWaitTime = 15;
int DCloudJob::maxRetryTimes = 3;

DCloudJob::DCloudJob( ClassAd *classad )
	: BaseJob( classad )
{
	char buff[16385]; // user data can be 16K, this is 16K+1
	MyString error_string = "";
	char *gahp_path = NULL;

	m_serviceUrl = NULL;
	m_instanceId = NULL;
	m_instanceName = NULL;
	m_imageId = NULL;
	m_realmId = NULL;
	m_hwpId = NULL;
	m_username = NULL;
	m_password = NULL;

	remoteJobState = "";
	gmState = GM_INIT;
	probeNow = false;
	enteredCurrentGmState = time(NULL);
	lastSubmitAttempt = 0;
	numSubmitAttempts = 0;
	myResource = NULL;
	gahp = NULL;

	// In GM_HOLD, we assume HoldReason to be set only if we set it, so make
	// sure it's unset when we start (unless the job is already held).
	if ( condorState != HELD && jobAd->LookupString( ATTR_HOLD_REASON, NULL, 0 ) != 0 ) {
		jobAd->AssignExpr( ATTR_HOLD_REASON, "Undefined" );
	}

	gahp_path = param( "DCLOUD_GAHP" );
	if ( gahp_path == NULL ) {
		error_string = "DCLOUD_GAHP not defined";
		goto error_exit;
	}

	gahp = new GahpClient( DCLOUD_RESOURCE_NAME, gahp_path );
	free(gahp_path);
	gahp->setNotificationTimerId( evaluateStateTid );
	gahp->setMode( GahpClient::normal );
	gahp->setTimeout( gahpCallTimeout );

	buff[0] = '\0';
	jobAd->LookupString( ATTR_GRID_RESOURCE, buff );
	if ( buff[0] ) {
		const char *token;
		MyString str = buff;

		str.Tokenize();

		token = str.GetNextToken( " ", false );
		if ( !token || strcasecmp( token, "dcloud" ) ) {
			error_string.sprintf( "%s not of type dcloud",
								  ATTR_GRID_RESOURCE );
			goto error_exit;
		}

		token = str.GetNextToken( " ", false );
		if ( token ) {
			m_serviceUrl = strdup( token );
		}

		token = str.GetNextToken( " ", false );
		if ( token ) {
			m_username = strdup( token );
		}

		token = str.GetNextToken( " ", false );
		if ( token ) {
			m_password = strdup( token );
		}

		token = str.GetNextToken( " ", false );
		if ( token ) {
			m_imageId = strdup( token );
		}

		token = str.GetNextToken( " ", false );
		if ( token ) {
			m_instanceName = strdup( token );
		}

		token = str.GetNextToken( " ", false );
		if ( token ) {
			m_realmId = strdup( token );
		}

		token = str.GetNextToken( " ", false );
		if ( token ) {
			m_hwpId = strdup( token );
		}

	}
	if ( m_realmId == NULL ) {
		error_string.sprintf( "%s missing or incomplete", ATTR_GRID_RESOURCE );
		goto error_exit;
	}

	buff[0] = '\0';
	jobAd->LookupString( ATTR_GRID_JOB_ID, buff );
	if ( buff[0] ) {
		const char *token;
		MyString str = buff;

		str.Tokenize();

		token = str.GetNextToken( " ", false );
		if ( !token || strcasecmp( token, "dcloud" ) ) {
			error_string.sprintf( "%s not of type dcloud",
								  ATTR_GRID_JOB_ID );
			goto error_exit;
		}

		token = str.GetNextToken( " ", false );
		if ( token ) {
			SetInstanceName( token );
		}

		token = str.GetNextToken( " ", false );
		if ( token ) {
			SetInstanceId( token );
		}
	}

	myResource = DCloudResource::FindOrCreateResource( m_serviceUrl, m_username, m_password );
	myResource->RegisterJob( this );
	if ( m_instanceId ) {
		myResource->AlreadySubmitted( this );
	}

	jobAd->LookupString( ATTR_GRID_JOB_STATUS, remoteJobState );

	return;

 error_exit:
	gmState = GM_HOLD;
	if ( !error_string.IsEmpty() ) {
		jobAd->Assign( ATTR_HOLD_REASON, error_string.Value() );
	}

	return;
}

DCloudJob::~DCloudJob()
{
	if ( myResource ) myResource->UnregisterJob( this );

	if ( gahp != NULL ) delete gahp;

	if ( m_instanceId ) {
		MyString hashname;
		hashname.sprintf( "%s#%s", m_serviceUrl, m_instanceId );
		JobsByInstanceId.insert( HashKey( hashname.Value() ), this );
	}

	free( m_serviceUrl );
	free( m_instanceId );
	free( m_instanceName );
	free( m_imageId );
	free( m_realmId );
	free( m_hwpId );
	free( m_username );
	free( m_password );
}


void DCloudJob::Reconfig()
{
	BaseJob::Reconfig();
}


void DCloudJob::doEvaluateState()
{
	int old_gm_state;
	bool reevaluate_state = true;
	time_t now = time(NULL);

	bool attr_exists;
	bool attr_dirty;
	int rc;

	daemonCore->Reset_Timer( evaluateStateTid, TIMER_NEVER );

    dprintf(D_ALWAYS, "(%d.%d) doEvaluateState called: gmState %s, condorState %d\n",
			procID.cluster,procID.proc,GMStateNames[gmState],condorState);

	if ( gahp ) {
		if ( !resourceStateKnown || resourcePingPending || resourceDown ) {
			gahp->setMode( GahpClient::results_only );
		} else {
			gahp->setMode( GahpClient::normal );
		}
	}

	do {

		char *gahp_error_code = NULL;

		reevaluate_state = false;
		old_gm_state = gmState;

		switch ( gmState ) 
		{
			case GM_INIT:
				// This is the state all jobs start in when the DCloudJob object
				// is first created. Here, we do things that we didn't want to
				// do in the constructor because they could block (the
				// constructor is called while we're connected to the schedd).
				if ( gahp->Startup() == false ) {
					dprintf( D_ALWAYS, "(%d.%d) Error starting GAHP\n", procID.cluster, procID.proc );
					jobAd->Assign( ATTR_HOLD_REASON, "Failed to start GAHP" );
					gmState = GM_HOLD;
					break;
				}

				gmState = GM_START;
				break;

			case GM_START:

				errorString = "";

				if ( m_instanceName == NULL || strcmp(m_instanceName, "NULL") == 0) {
					gmState = GM_CLEAR_REQUEST;
				} else if ( m_instanceId == NULL ) {
					gmState = GM_CHECK_VM;
				} else {
					submitLogged = true;
					if ( condorState == RUNNING || condorState == COMPLETED ) {
						executeLogged = true;
					}
					gmState = GM_SUBMITTED;
				}

				break;

			case GM_CHECK_VM: {
				char *instance_id = NULL;
				// check if the VM has been started successfully
				rc = gahp->dcloud_find( m_serviceUrl,
										m_username,
										m_password,
										m_instanceName,
										&instance_id );
				if ( rc == GAHPCLIENT_COMMAND_NOT_SUBMITTED ||
					 rc == GAHPCLIENT_COMMAND_PENDING ) {
					break;
				}

				if (rc == 0) {
					if ( instance_id && *instance_id ) {
						SetInstanceId( instance_id );
						free( instance_id );
						myResource->AlreadySubmitted( this );
						probeNow = true;
						gmState = GM_SAVE_INSTANCE_ID;
					} else {
						gmState = GM_CREATE_VM;
					}
				} else {
					errorString = gahp->getErrorString();
					dprintf(D_ALWAYS,"(%d.%d) VM check failed: %s: %s\n",
							procID.cluster, procID.proc, gahp_error_code,
							errorString.Value() );
					gmState = GM_HOLD;
				}

				} break;

			case GM_UNSUBMITTED:

				if ( (condorState == REMOVED) || (condorState == HELD) ) {
					gmState = GM_DELETE;
				} else {
					gmState = GM_SAVE_INSTANCE_NAME;
				}

				break;

			case GM_SAVE_INSTANCE_NAME:
				// Create a unique name for this job
				// and save it in GridJobId in the schedd. This
				// will be our handle to the job until we get the instance
				// id at the end of the submission process.

				if ( (condorState == REMOVED) ||
					 (condorState == HELD) ) {

					gmState = GM_DELETE;
				}

				// Once RequestSubmit() is called at least once, you must
				// CancelSubmit() once the submission process is complete
				// or aborted.
				if ( myResource->RequestSubmit( this ) == false ) {
					break;
				}

				if ( m_instanceName == NULL || strcmp(m_instanceName, "NULL") == 0) {
					SetInstanceName( build_instance_name().Value() );
				}
				jobAd->GetDirtyFlag( ATTR_GRID_JOB_ID, &attr_exists, &attr_dirty );
				if ( attr_exists && attr_dirty ) {
						// The instance name still needs to be saved to
						//the schedd
					requestScheddUpdate( this, true );
					break;
				}
				gmState = GM_CREATE_VM;
				break;

			case GM_CREATE_VM:

				if ( numSubmitAttempts >= MAX_SUBMIT_ATTEMPTS ) {
					gmState = GM_HOLD;
					break;
				}

				// After a submit, wait at least submitInterval before trying another one.
				if ( now >= lastSubmitAttempt + submitInterval ) {

					// Once RequestSubmit() is called at least once, you must
					// CancelSubmit() once you're done with the request call
					if ( myResource->RequestSubmit( this ) == false ) {
						// If we haven't started the START_VM call yet,
						// we can abort the submission here for held and
						// removed jobs.
						if ( (condorState == REMOVED) ||
							 (condorState == HELD) ) {

							myResource->CancelSubmit( this );
							gmState = GM_STOPPED;
						}
						break;
					}

					StringList instance_attrs;

					rc = gahp->dcloud_submit( m_serviceUrl,
											  m_username,
											  m_password,
											  m_imageId,
											  m_instanceName,
											  m_realmId,
											  m_hwpId,
											  instance_attrs );
					if ( rc == GAHPCLIENT_COMMAND_NOT_SUBMITTED ||
						 rc == GAHPCLIENT_COMMAND_PENDING ) {
						break;
					}

					lastSubmitAttempt = time(NULL);

					if ( rc == 0 ) {

						ProcessInstanceAttrs( instance_attrs );
						ASSERT( m_instanceId );
						WriteGridSubmitEventToUserLog(jobAd);

						if ( remoteJobState == DCLOUD_VM_STATE_STOPPED ) {
							gmState = GM_START_VM;
						} else {
							gmState = GM_SAVE_INSTANCE_ID;
						}

					 } else {
						errorString = gahp->getErrorString();
						dprintf(D_ALWAYS,"(%d.%d) job submit failed: %s: %s\n",
								procID.cluster, procID.proc, gahp_error_code,
								errorString.Value() );
						gmState = GM_HOLD;
					}

				} else {
					if ( (condorState == REMOVED) || (condorState == HELD) ) {
						gmState = GM_STOPPED;
						break;
					}

					unsigned int delay = 0;
					if ( (lastSubmitAttempt + submitInterval) > now ) {
						delay = (lastSubmitAttempt + submitInterval) - now;
					}				
					daemonCore->Reset_Timer( evaluateStateTid, delay );
				}

				break;


			case GM_START_VM:

				rc = gahp->dcloud_action( m_serviceUrl,
										  m_username,
										  m_password,
										  m_instanceId,
										  "start" );
				if ( rc == GAHPCLIENT_COMMAND_NOT_SUBMITTED ||
					 rc == GAHPCLIENT_COMMAND_PENDING ) {
					break;
				} 

				if ( rc == 0 ) {
					gmState = GM_SAVE_INSTANCE_ID;
				} else {
					// What to do about a failed start?
					errorString = gahp->getErrorString();
					dprintf( D_ALWAYS, "(%d.%d) job start failed: %s: %s\n",
							 procID.cluster, procID.proc, gahp_error_code,
							 errorString.Value() );
					gmState = GM_HOLD;
				}
				break;

			case GM_SAVE_INSTANCE_ID:

				jobAd->GetDirtyFlag( ATTR_GRID_JOB_ID, &attr_exists, &attr_dirty );
				if ( attr_exists && attr_dirty ) {
					// Wait for the instance id to be saved to the schedd
					requestScheddUpdate( this, true );
					break;
				}					
				gmState = GM_SUBMITTED;

				break;


			case GM_SUBMITTED:

				// TODO Make sure instances that begin in the 'stopped'
				//   state aren't flagged here.
				if ( remoteJobState == DCLOUD_VM_STATE_FINISH ||
					 remoteJobState == DCLOUD_VM_STATE_STOPPED ) {

					gmState = GM_DONE_SAVE;

				} else if ( condorState == REMOVED || condorState == HELD ) {

					gmState = GM_CANCEL;

				} else if ( probeNow ) {
					gmState = GM_PROBE_JOB;
					break;
				}

				break;


			case GM_DONE_SAVE:

				if ( condorState != HELD && condorState != REMOVED ) {
					JobTerminated();
					if ( condorState == COMPLETED ) {
						jobAd->GetDirtyFlag( ATTR_JOB_STATUS, &attr_exists, &attr_dirty );
						if ( attr_exists && attr_dirty ) {
							requestScheddUpdate( this, true );
							break;
						}
					}
				}

				myResource->CancelSubmit( this );
				if ( condorState == COMPLETED || condorState == REMOVED ) {
					gmState = GM_STOPPED;
				} else {
					// Clear the contact string here because it may not get
					// cleared in GM_CLEAR_REQUEST (it might go to GM_HOLD first).
					if ( m_instanceId != NULL ) {
						SetInstanceId( NULL );
						SetInstanceName( NULL );
					}
					gmState = GM_CLEAR_REQUEST;
				}

				break;


			case GM_CLEAR_REQUEST:

				// Remove all knowledge of any previous or present job
				// submission, in both the gridmanager and the schedd.

				// If we are doing a rematch, we are simply waiting around
				// for the schedd to be updated and subsequently this dcloud job
				// object to be destroyed.  So there is nothing to do.
				if ( wantRematch ) {
					break;
				}

				// For now, put problem jobs on hold instead of
				// forgetting about current submission and trying again.
				// TODO: Let our action here be dictated by the user preference
				// expressed in the job ad.
				if ( m_instanceId != NULL && condorState != REMOVED 
					 && wantResubmit == 0 && doResubmit == 0 ) {
					gmState = GM_HOLD;
					break;
				}

				// Only allow a rematch *if* we are also going to perform a resubmit
				if ( wantResubmit || doResubmit ) {
					jobAd->EvalBool(ATTR_REMATCH_CHECK,NULL,wantRematch);
				}

				if ( wantResubmit ) {
					wantResubmit = 0;
					dprintf(D_ALWAYS, "(%d.%d) Resubmitting to DCloud because %s==TRUE\n",
						procID.cluster, procID.proc, ATTR_GLOBUS_RESUBMIT_CHECK );
				}

				if ( doResubmit ) {
					doResubmit = 0;
					dprintf(D_ALWAYS, "(%d.%d) Resubmitting to DCloud (last submit failed)\n",
						procID.cluster, procID.proc );
				}

				errorString = "";
				myResource->CancelSubmit( this );
				if ( m_instanceId != NULL ) {
					SetInstanceId( NULL );
					SetInstanceName( NULL );
				}

				JobIdle();

				if ( submitLogged ) {
					JobEvicted();
					if ( !evictLogged ) {
						WriteEvictEventToUserLog( jobAd );
						evictLogged = true;
					}
				}

				if ( wantRematch ) {
					dprintf(D_ALWAYS, "(%d.%d) Requesting schedd to rematch job because %s==TRUE\n",
						procID.cluster, procID.proc, ATTR_REMATCH_CHECK );

					// Set ad attributes so the schedd finds a new match.
					int dummy;
					if ( jobAd->LookupBool( ATTR_JOB_MATCHED, dummy ) != 0 ) {
						jobAd->Assign( ATTR_JOB_MATCHED, false );
						jobAd->Assign( ATTR_CURRENT_HOSTS, 0 );
					}

					// If we are rematching, we need to forget about this job
					// cuz we wanna pull a fresh new job ad, with a fresh new match,
					// from the all-singing schedd.
					gmState = GM_DELETE;
					break;
				}

				// If there are no updates to be done when we first enter this
				// state, requestScheddUpdate will return done immediately
				// and not waste time with a needless connection to the
				// schedd. If updates need to be made, they won't show up in
				// schedd_actions after the first pass through this state
				// because we modified our local variables the first time
				// through. However, since we registered update events the
				// first time, requestScheddUpdate won't return done until
				// they've been committed to the schedd.
				const char *name;
				ExprTree *expr;
				jobAd->ResetExpr();
				if ( jobAd->NextDirtyExpr(name, expr) ) {
					requestScheddUpdate( this, true );
					break;
				}

				if ( remoteJobState != "" ) {
					remoteJobState = "";
					SetRemoteJobStatus( NULL );
				}

				submitLogged = false;
				executeLogged = false;
				submitFailedLogged = false;
				terminateLogged = false;
				abortLogged = false;
				evictLogged = false;
				gmState = GM_UNSUBMITTED;

				break;				

			case GM_PROBE_JOB:

				probeNow = false;
				if ( condorState == REMOVED || condorState == HELD ) {
					gmState = GM_CANCEL;
				} else {

					StringList attrs;
					rc = gahp->dcloud_info( m_serviceUrl,
											m_username,
											m_password,
											m_instanceId,
											attrs );
					if ( rc == GAHPCLIENT_COMMAND_NOT_SUBMITTED ||
						 rc == GAHPCLIENT_COMMAND_PENDING ) {
						break;
					}

					// processing error code received
					if ( rc != 0 ) {
						// What to do about failure?
						errorString = gahp->getErrorString();
						dprintf( D_ALWAYS, "(%d.%d) job probe failed: %s: %s\n",
								 procID.cluster, procID.proc, gahp_error_code,
								 errorString.Value() );
						gmState = GM_HOLD;
						break;
					}

					ProcessInstanceAttrs( attrs );

					gmState = GM_SUBMITTED;
				}

				break;				

			case GM_CANCEL:

				rc = gahp->dcloud_action( m_serviceUrl,
										  m_username,
										  m_password,
										  m_instanceId,
										  "stop" );
				if ( rc == GAHPCLIENT_COMMAND_NOT_SUBMITTED ||
					 rc == GAHPCLIENT_COMMAND_PENDING ) {
					break;
				} 

				if ( rc == 0 ) {
					gmState = GM_STOPPED;
				} else {
					// What to do about a failed cancel?
					errorString = gahp->getErrorString();
					dprintf( D_ALWAYS, "(%d.%d) job cancel failed: %s: %s\n",
							 procID.cluster, procID.proc, gahp_error_code,
							 errorString.Value() );
					gmState = GM_HOLD;
				}
				break;

			case GM_HOLD:
				// Put the job on hold in the schedd.
				// If the condor state is already HELD, then someone already
				// HELD it, so don't update anything else.
				if ( condorState != HELD ) {

					// Set the hold reason as best we can
					// TODO: set the hold reason in a more robust way.
					char holdReason[1024];
					holdReason[0] = '\0';
					holdReason[sizeof(holdReason)-1] = '\0';
					jobAd->LookupString( ATTR_HOLD_REASON, holdReason, sizeof(holdReason) - 1 );
					if ( holdReason[0] == '\0' && errorString != "" ) {
						strncpy( holdReason, errorString.Value(), sizeof(holdReason) - 1 );
					} else if ( holdReason[0] == '\0' ) {
						strncpy( holdReason, "Unspecified gridmanager error", sizeof(holdReason) - 1 );
					}

					JobHeld( holdReason );
				}

				gmState = GM_DELETE;

				break;


			case GM_STOPPED:

				if ( remoteJobState != DCLOUD_VM_STATE_FINISH ) {
// destroy action currently not supported by gahp
rc=0;
/*
					rc = gahp->dcloud_action( m_serviceUrl,
											  m_username,
											  m_password,
											  m_instanceId,
											  "destroy" );
*/
					if ( rc == GAHPCLIENT_COMMAND_NOT_SUBMITTED ||
						 rc == GAHPCLIENT_COMMAND_PENDING ) {
						break;
					}

					if ( rc == 0 ) {
						StatusUpdate( DCLOUD_VM_STATE_FINISH );
					} else {
						// What to do about a failed destroy?
						errorString = gahp->getErrorString();
						dprintf( D_ALWAYS, "(%d.%d) job destroy failed: %s: %s\n",
								 procID.cluster, procID.proc, gahp_error_code,
								 errorString.Value() );
						gmState = GM_HOLD;
						break;
					}
				}
				myResource->CancelSubmit( this );
				SetInstanceId( NULL );
				SetInstanceName( NULL );

				if ( (condorState == REMOVED) || (condorState == COMPLETED) ) {
					gmState = GM_DELETE;
				} else {
					gmState = GM_CLEAR_REQUEST;
				}

				break;

			case GM_DELETE:

				// We are done with the job. Propagate any remaining updates
				// to the schedd, then delete this object.
				DoneWithJob();
				// This object will be deleted when the update occurs
				break;							


			default:
				EXCEPT( "(%d.%d) Unknown gmState %d!", procID.cluster, procID.proc, gmState );
				break;
		} // end of switch_case

			// This string is used for gahp calls, but is never needed beyond
			// this point. This should really be a MyString.
		free( gahp_error_code );
		gahp_error_code = NULL;

		if ( gmState != old_gm_state ) {
			reevaluate_state = true;
			dprintf(D_FULLDEBUG, "(%d.%d) gm state change: %s -> %s\n",
					procID.cluster, procID.proc, GMStateNames[old_gm_state], GMStateNames[gmState]);
			enteredCurrentGmState = time(NULL);
		}

	} // end of do_while
	while ( reevaluate_state );	
}


BaseResource* DCloudJob::GetResource()
{
	return (BaseResource *)myResource;
}


void DCloudJob::SetInstanceName( const char *instance_name )
{
	free( m_instanceName );
	if ( instance_name ) {
		m_instanceName = strdup( instance_name );
	} else {
		m_instanceName = NULL;
	}
	SetRemoteJobId( m_instanceName, m_instanceId );
}

void DCloudJob::SetInstanceId( const char *instance_id )
{
	MyString hashname;
	if ( m_instanceId ) {
		hashname.sprintf( "%s#%s", m_serviceUrl, m_instanceId );
		JobsByInstanceId.remove( HashKey( hashname.Value() ) );
		free( m_instanceId );
	}
	if ( instance_id ) {
		m_instanceId = strdup( instance_id );
		hashname.sprintf( "%s#%s", m_serviceUrl, m_instanceId );
		JobsByInstanceId.insert( HashKey( hashname.Value() ), this );
	} else {
		m_instanceId = NULL;
	}
	SetRemoteJobId( m_instanceName, m_instanceId );
}

// SetRemoteJobId() is used to set the value of global variable "remoteJobID"
void DCloudJob::SetRemoteJobId( const char *instance_name, const char *instance_id )
{
	MyString full_job_id;
	if ( instance_name && instance_name[0] ) {
		full_job_id.sprintf( "dcloud %s", instance_name );
		if ( instance_id && instance_id[0] ) {
			full_job_id.sprintf_cat( " %s", instance_id );
		}
	}
	BaseJob::SetRemoteJobId( full_job_id.Value() );
}

void DCloudJob::ProcessInstanceAttrs( StringList &attrs )
{
	// TODO
	// NOTE: If attrlist is empty, treat as completed job
	// Call StatusUpdate() with status from list
	const char *line;
	attrs.rewind();
	while ( (line = attrs.next()) ) {
		if ( strncmp( line, "state=", 6 ) == 0 ) {
			StatusUpdate( &line[6] );
		} else if ( strncmp( line, "id=", 3 ) == 0 ) {
			SetInstanceId( &line[3] );
		}
	}

	if ( attrs.isEmpty() ) {
		StatusUpdate( DCLOUD_VM_STATE_FINISH );
	}
}

void DCloudJob::StatusUpdate( const char *new_status )
{
	if ( new_status == NULL ) {
			// TODO May need to prevent this firing if job was just
			//   submitted, so it's not in the query
		probeNow = true;
		SetEvaluateState();
	} else if ( SetRemoteJobStatus( new_status ) ) {
		// TODO Should 'shutting-down' be treated as running?
		if ( strcasecmp( new_status, DCLOUD_VM_STATE_RUNNING ) == 0 ) {
			JobRunning();
		}
		remoteJobState = new_status;
		probeNow = true;
		SetEvaluateState();
	}
}

MyString DCloudJob::build_instance_name()
{
	// Build a name that will be unique to this job.
	// Our pattern is Condor_<collector name>_<GlobalJobId>

	// get condor pool name
	// In case there are multiple collectors, strip out the spaces
	// If there's no collector, insert a dummy name
	char* pool_name = param( "COLLECTOR_HOST" );
	if ( pool_name ) {
		StringList collectors( pool_name );
		free( pool_name );
		pool_name = collectors.print_to_string();
	} else {
		pool_name = strdup( "NoPool" );
	}

	// use "ATTR_GLOBAL_JOB_ID" to get unique global job id
	MyString job_id;
	jobAd->LookupString( ATTR_GLOBAL_JOB_ID, job_id );

	MyString instance_name;
	instance_name.sprintf( "Condor_%s_%s", pool_name, job_id.Value() );

	free( pool_name );
	return instance_name;
}
