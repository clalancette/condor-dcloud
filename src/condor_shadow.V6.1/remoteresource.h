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


#ifndef REMOTERESOURCE_H
#define REMOTERESOURCE_H

#include "condor_common.h"
#include "condor_daemon_core.h"
#include "condor_daemon_client.h"
#include "condor_classad.h"
#include "reli_sock.h"
#include "baseshadow.h"
#include "file_transfer.h"
#include "condor_claimid_parser.h"

/** The states that a remote resource can be in.  If you add anything
	here you must A) put it before _RR_STATE_THRESHOLD and B) add a
	string to Resource_State_String in remoteresource.C */ 
typedef enum {
		/// Before the starter has been spawned
	RR_PRE, 
		/// While it's running
	RR_EXECUTING,
		/** We've told the job to go away, but haven't received 
			confirmation that it's really dead.  This state is 
		    skipped if the job exits on its own. */
	RR_PENDING_DEATH,
		/// After it has stopped (for whatever reason...)
	RR_FINISHED,
		/// Suspended at the execution site
	RR_SUSPENDED,
		/// Starter exists, but the job is not executing yet 
	RR_STARTUP,
		/// Trying to reconnect to the starter
	RR_RECONNECT,
		/// job is checkpointed
	RR_CHECKPOINTED,
		/// The threshold must be last!
	_RR_STATE_THRESHOLD
} ResourceState;

	/** Return the string version of the given ResourceState */
const char* rrStateToString( ResourceState s );

/** This class represents one remotely running user job.  <p>

	This class has a Socket to the remote job from which it can handle
	syscalls.  It can also talk to the startd and request to start a
	starter, and it can kill the starter as well.<p>

	For all functions that start with 'get' and are passed a char*, 
	the following applies: if a NULL value is passed in as the argument, 
	space is new'ed for you, *unless* the member you requested is NULL.  
	If it is NULL, space is not new'ed and a NULL is returned.  If the 
	parameter you pass is not NULL, it is assumed to point to valid memory, 
	and the char* will be copied in.  If the member you request is NULL, 
	the parameter you passed in will be set to "".<p>
	
    <b>Summary of get* functions:</b><p>
	<pre>
	Parameter  Member   Results
	---------  ------   ----------
	  NULL     exists    string strnewp'ed and returned
	  NULL      NULL     NULL returned
	 !NULL     exists    strcpy occurs
	 !NULL      NULL     "" into parameter
	</pre>

	More to come...<p>

	@author Mike Yoder
*/
class RemoteResource : public Service {

 public: 

		/** Constructor.  Pass it a pointer to the shadow which creates
			it, this is handy while in handleSysCalls.  
		*/
	RemoteResource( BaseShadow *shadow );

		/// Destructor
	virtual ~RemoteResource();

		/** This function connects to the executing host and does
			an ACTIVATE_CLAIM command on it.  The ClaimId, starternum
			and Job ClassAd are pushed, and the executing host's 
			full machine name and (hopefully) an OK are returned.
			@param starterVersion The version number of the starter
                   wanted. The default is 2.
			@return true on success, false on failure
		 */ 
	bool activateClaim( int starterVersion = 2 );

		/** Tell the remote starter to kill itself.
			@param graceful Should we do a graceful or fast shutdown?
			@return true on success, false if a problem occurred.
		*/
	virtual bool killStarter( bool graceful = false );

		/** Print out this representation of the remote resource.
			@param debugLevel The dprintf debug level you wish to use 
		*/
	virtual void dprintfSelf( int debugLevel);

		/** Each of these remote resources can handle syscalls.  The 
			claimSock gets registered with daemonCore, and it will
			call this function to handle it.
			@param sock Stuff comes in here
			@return KEEP_STREAM
		*/
	virtual int handleSysCalls( Stream *sock );

		/** Return the machine name of the remote host.
			@param machineName Will contain the host's machine name.
		*/ 
	void getMachineName( char *& machineName );

		/** Return the filesystem domain of the remote host.
			@param filesystemDomain Will contain the host's fs_domain
		*/
	void getFilesystemDomain( char *& filesystemDomain );

		/** Return the uid domain of the remote host.
			@param uidDomain Will contain the host's uid_domain
		*/
	void getUidDomain( char *& uidDomain );

		/** Return the sinful string of the starter.
			@param starterAddr Will contain the starter's sinful string.
		*/
	void getStarterAddress( char *& starterAddr );

		/** Return the sinful string of the remote startd.
			@param sinful Will contain the host's sinful string.  If
			NULL, this will be a string allocated with new().  If
			sinful already exists, we assume it's a buffer and print
			into it.
       */ 
   void getStartdAddress( char *& sinful );

		/** Return the name of the remote startd.
			@param remote_name Will contain the host's name.  If
			NULL, this will be a string allocated with new().  If
			remote_name already exists, we assume it's a buffer and print
			into it.
       */ 
   void getStartdName( char *& remote_name );

		/** Return the ClaimId string of the remote startd.
			@param id Will contain the ClaimId string.  If NULL,
			this will be a string allocated with new().  If id
			already exists, we assume it's a buffer and print into it.
       */
   void getClaimId( char *& id );

		/** Return the arch string of the starter.
			@param arch Will contain the starter's arch string.
		*/
	void getStarterArch( char *& arch );

		/** Return the opsys string of the starter.
			@param opsys Will contain the starter's opsys string.
		*/
	void getStarterOpsys( char *& opsys );

		/** Return the claim socket associated with this remote host.  
			@return The claim socket for this host.
		*/ 
	ReliSock* getClaimSock();

		/** Called when our syscall socket got closed.  So, cancel the
			daemoncore socket handler, delete the object, and set our
			pointer to NULL.
		*/
	void closeClaimSock( void );

		/** Return the reason this host exited.
			@return The exit reason for this host.
		*/ 
	int  getExitReason();

		/** @return true if startd is not accepting more jobs on this claim.
		*/ 
	bool claimIsClosing();

		/** Return a pointer to our DCStartd object, so callers can
			access information in there directly, without having to
			add other methods in here...
		*/ 
	DCStartd* getDCStartd() { return dc_startd; };

		/** Set the info about the startd associated with this
			remote resource via attributes in the given ClassAd
		*/
	void setStartdInfo( ClassAd* ad );

		/** Set the sinful string and ClaimId for the startd
			associated with this remote resource.
			This method is deprecated, use the version that takes a
			ClassAd if possible.
			@param sinful The sinful string for the startd
			@param claim_id The ClaimId string for the startd
		*/
	void setStartdInfo( const char *sinful, const char* claim_id );

		/** Set all the info about the starter we can find in the
			given ClassAd. 
		*/
	void setStarterInfo( ClassAd* ad );

		/** If we are generating security sessions from the match
			info (claim id), then this function provides the information
			needed by the starter.
		*/
	bool getSecSessionInfo(
		char const *starter_reconnect_session_info,
		MyString &reconnect_session_id,
		MyString &reconnect_session_info,
		MyString &reconnect_session_key,
		char const *starter_filetrans_session_info,
		MyString &filetrans_session_id,
		MyString &filetrans_session_info,
		MyString &filetrans_session_key);

		/** Set the reason this host exited.  
			@param reason Why did it exit?  Film at 11.
		*/
	virtual void setExitReason( int reason );

		/** Set this resource's jobAd, and initialize any relevent
			local variables from the ad if the attributes are there.
		*/
	void setJobAd( ClassAd *jA );

		/** Get this resource's jobAd */
	ClassAd* getJobAd() { return this->jobAd; };

		/** Set the address (sinful string).
			@param The starter's sinful string 
		*/
	void setStarterAddress( const char *starterAddr );

		/**
		   CRUFT the next 3 methods for setting info are only used if
		   we're talking to a really old version of the starter.
		   someday they should all be ripped out in favor of using
		   setStarterInfo(), declared above...
		*/

		/** Set the machine name for this host.  CRUFT
			@param machineName The machine name of this host.
		*/
	void setMachineName( const char *machineName );

		/** Set the filesystem domain for this host.  CRUFT
			@param filesystemDomain The filesystem domain of this host.
		*/
	void setFilesystemDomain( const char *filesystemDomain );

		/** Set the uid domain for this host.  CRUFT
			@param uidDomain The uid domain of this host.
		*/
	void setUidDomain( const char *uidDomain );

		/// The number of bytes sent to this resource.
	float bytesSent();
		
		/// The number of bytes received from this resource.
	float bytesReceived();

	FileTransfer filetrans;

	virtual void resourceExit( int reason_for_exit, int exit_status );

	virtual void updateFromStarter( ClassAd* update_ad );

	int getImageSize( void ) { return image_size; };
	int getDiskUsage( void ) { return disk_usage; };
	struct rusage getRUsage( void ) { return remote_rusage; };

		/** Return the state that this resource is in. */
	ResourceState getResourceState() { return state; };

		/** Change this resource's state */
	void setResourceState( ResourceState s );

		/** Record the fact that our starter suspended the job.  This
			includes updating our in-memory job ad, logging an event
			to the UserLog, etc.
			@param update_ad ClassAd with update info from the starter
			@return true on success, false on failure
		*/
	virtual bool recordSuspendEvent( ClassAd* update_ad );

		/** Record the fact that our starter resumed the job.  This
			includes updating our in-memory job ad, logging an event
			to the UserLog, etc.
			@param update_ad ClassAd with update info from the starter
			@return true on success, false on failure
		*/
	virtual bool recordResumeEvent( ClassAd* update_ad );

		/** Record the fact that our starter checkpointed the job.  This
			includes updating our in-memory job ad, logging an event
			to the UserLog, etc.
			@param update_ad ClassAd with update info from the starter
			@return true on success, false on failure
		*/
	virtual bool recordCheckpointEvent( ClassAd* update_ad );

		/** Write the given event to the UserLog.  This is virtual so
			each kind of RemoteResource can define its own version.
			@param event Pointer to ULogEvent to write
			@return true on success, false on failure
		*/
	virtual bool writeULogEvent( ULogEvent* event );

		/// Did the job on this resource exit with a signal?
	bool exitedBySignal( void ) { return exited_by_signal; };

		/** If the job on this resource exited with a signal, return
			the signal.  If not, return -1. */
	int exitSignal( void );

		/** If the job on this resource exited normally, return the
			exit code.  If it was killed by a signal, return -1. */ 
	int exitCode( void );

		/** This method is called when the job at the remote resource
			has finally started to execute.  We use this to log the
			appropriate user log event(s), start various timers, etc. 
		*/
	virtual void beginExecution( void );

	virtual void reconnect( void );

	virtual bool supportsReconnect( void );

		/** This is actually a DaemonCore timer handler.  It has to be
			public so DaemonCore can use it, but it's not meant to be 
			called by other parts of the shadow.
		*/
	virtual void attemptReconnect( void );

		/// If this resource has a lease, how much time until it expires?
	virtual int remainingLeaseDuration( void );

		/** Check if the X509 has been updated, if so upload it to the shadow

			Intended for use as a DaemonCore timer handler (thus, it's public),
			but should be safe if you really feel like calling it yourself.
		*/
	virtual void checkX509Proxy( void );


		// Try to send an updated X509 proxy down to the starter
	bool updateX509Proxy(const char * filename);

	// return true if job should be allowed to read from filename
	bool allowRemoteReadFileAccess( char const * filename );

	// return true if job should be allowed to write to filename
	bool allowRemoteWriteFileAccess( char const * filename );

	// return true if job should be allowed to read from attribute
	bool allowRemoteReadAttributeAccess( char const * name );

	// return true if job should be allowed to write to attribute
	bool allowRemoteWriteAttributeAccess( char const * name );

 protected:

		/** The jobAd for this resource.  Why is this here and not
			in the shadow?  Well...if we're an MPI job, say, and we
			want the different nodes to have a slightly different ad
			for things like i/o, etc, we have to have one copy of 
			the ClassAd for each resource...and thus, it's here. */
	ClassAd *jobAd;

		/* internal data: if you can't figure the following out.... */
	char *machineName;
	char *starterAddress;
	char *starterArch;
	char *starterOpsys;
	char *starter_version;
	char *fs_domain;
	char *uid_domain;
	ReliSock *claim_sock;
	int exit_reason;
	bool claim_is_closing;
	int exit_value;
	bool exited_by_signal;

	bool m_want_chirp;
	bool m_want_streaming_io;

		// If we specially create a security session for file transfer,
		// this records all the information we need to know about it.
		// We use the ClaimIdParser class for convenience, because it
		// already handles storing this kind of data.
	ClaimIdParser m_filetrans_session;

		// If we specially create a security session for this claim
		// (SEC_ENABLE_MATCH_PASSWORD_AUTHENTICATION=TRUE), then this records all the
		// information we need to know about it.  This is literally
		// just a copy of the claim id string.  Many of the uses of
		// this claim session happen implicitly in the DCStartd
		// class, which has its own copy of the claim id.
	ClaimIdParser m_claim_session;

		/// Updates both the last_contact data member and the job ad
	virtual void hadContact( void );
	time_t last_job_lease_renewal;
	bool supports_reconnect;

		// More resource-specific stuff goes here.

		// we keep a pointer to the shadow class around.  This is useful
		// in handleSysCalls.
	BaseShadow *shadow;
	
		// Info about the job running on this particular remote
		// resource:  
	struct rusage	remote_rusage;
	int 			image_size;
	int 			disk_usage;

	DCStartd* dc_startd;

		/** Initialize all the info about the startd associated with
			this remote resource, instantiate the DCStartd object, 
			grant ACLs for the execute host, etc.  This is called by
			setStartdInfo() to do the real work.
		*/
	void initStartdInfo( const char *name, const char* pool,
						 const char *addr, const char* claim_id );

	ResourceState state;

	bool began_execution;

private:

		/// Private helper methods for trying to reconnect
	bool locateReconnectStarter( void );
	void requestReconnect( void );
	int reconnect_attempts;
	int next_reconnect_tid;
	int proxy_check_tid;

	MyString proxy_path;
	time_t last_proxy_timestamp;

	time_t m_remote_proxy_expiration;
	time_t m_remote_proxy_renew_time;

		/** For debugging, print out the values of various statistics
			related to our bookkeeping of suspend/resume activity for
			the job.
			@param debug_level What dprintf level to use
		*/
	void printSuspendStats( int debug_level );
		/** For debugging, print out the values of various statistics
			related to our bookkeeping of checkpointing activity for
			the job.
			@param debug_level What dprintf level to use
		*/
	void printCheckpointStats( int debug_level );

		// Making these private PREVENTS copying.
	RemoteResource( const RemoteResource& );
	RemoteResource& operator = ( const RemoteResource& );

	int lease_duration;

	bool already_killed_graceful;
	bool already_killed_fast;

	void logRemoteAccessCheck(bool allow,char const *op,char const *name);

	void setRemoteProxyRenewTime(time_t expiration_time);
	void setRemoteProxyRenewTime();
	void startCheckingProxy();

};


#endif


