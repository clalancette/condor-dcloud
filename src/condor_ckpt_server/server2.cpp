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
#include "condor_debug.h"
#include "condor_config.h"
#include "condor_adtypes.h"
#include "condor_attributes.h"
#include "xfer_summary.h"
#include "string_list.h"
#include "internet.h"
#include "my_hostname.h"
#include "condor_version.h"
#include "condor_socket_types.h"
#include "subsystem_info.h"
#include "condor_netdb.h"
#include "condor_fix_iostream.h"
#include "condor_fix_fstream.h"

#include "server2.h"
#include "gen_lib.h"
#include "network2.h"
#include "signal2.h"
#include "alarm2.h"
#include "condor_uid.h"
#include "classad_collection.h"
#include "daemon.h"

XferSummary	xfer_summary;

Server server;
Alarm  rt_alarm;
#ifdef WANT_NETMAN
static bool ManageBandwidth = false;
static int NetworkHorizon = 300;
#endif

/* For daemonCore, etc. */
DECL_SUBSYSTEM( "CKPT_SERVER", SUBSYSTEM_TYPE_DAEMON );

char* myName = NULL;

extern "C" {
ssize_t stream_file_xfer( int src_fd, int dst_fd, size_t n_bytes );
int tcp_accept_timeout( int, struct sockaddr*, int*, int );
}

/* Ensure the checkpoint's filename I'm about to read or write stays in
	the checkpointing directory. */
int ValidateNoPathComponents(char *path);


Server::Server()
{
	more = 1;
	req_ID = 0;
		// We can't initialize this until Server::Init(), after
		// config() has been called, so we get NETWORK_INTERFACE. 
	server_addr.s_addr = 0;
	store_req_sd = -1;
	restore_req_sd = -1;
	service_req_sd = -1;
	replicate_req_sd = -1;
	socket_bufsize = 0;
	max_xfers = 0;
	max_store_xfers = 0;
	max_restore_xfers = 0;
	max_replicate_xfers = 0;
	num_peers = 0;
	CkptClassAds = NULL;
	clean_interval = 0;
	memset(peer_addr_list, 0, sizeof(peer_addr_list));
	max_req_sd_plus1 = 0;
	num_replicate_xfers = 0;
	num_restore_xfers = 0;
	num_store_xfers = 0;
	reclaim_interval = 0;
	replication_level = 0;
	check_parent_interval = 0;
}


Server::~Server()
{
	if (store_req_sd >= 0)
		close(store_req_sd);
	if (restore_req_sd >=0)
		close(restore_req_sd);
	if (service_req_sd >=0)
		close(service_req_sd);
	if (replicate_req_sd >=0)
		close(replicate_req_sd);
	delete CkptClassAds;
}


void Server::Init()
{
	char		*ckpt_server_dir, *collection_log;
	char        log_msg[256];
	char        hostname[100];
	int         buf_size;
	SOCKET_LENGTH_TYPE	len;
	static bool first_time = true;
	
	num_store_xfers = 0;
	num_restore_xfers = 0;
	num_replicate_xfers = 0;

	config();
	dprintf_config( mySubSystem->getName() );

	set_condor_priv();

		// We have to do this after we call config, not in the Server
		// constructor, or we won't have NETWORK_INTERFACE yet.
        // Commented out the following line so that server_addr is determined after
        // the socket is bound to an address. This is a part of making Condor GCB
        // friendly. -- Sonny 5/18/2005
	//server_addr.s_addr = htonl( my_ip_addr() );

	dprintf( D_ALWAYS,
			 "******************************************************\n" );
	dprintf( D_ALWAYS, "** %s (CONDOR_%s) STARTING UP\n", myName, 
			 mySubSystem->getName() );
	dprintf( D_ALWAYS, "** %s\n", CondorVersion() );
	dprintf( D_ALWAYS, "** %s\n", CondorPlatform() );
	dprintf( D_ALWAYS, "** PID = %lu\n", (unsigned long) getpid() );
	dprintf( D_ALWAYS,
			 "******************************************************\n" );

	ckpt_server_dir = param( "CKPT_SERVER_DIR" );
	if( ckpt_server_dir ) {
		if (chdir(ckpt_server_dir) < 0) {
			dprintf(D_ALWAYS, "Failed to chdir() to %s\n", ckpt_server_dir);
			exit(1);
		}
		dprintf(D_ALWAYS, "CKPT_SERVER running in directory %s\n", 
				ckpt_server_dir);
		free( ckpt_server_dir );
	}

	replication_level = param_integer( "CKPT_SERVER_REPLICATION_LEVEL",0 );
	reclaim_interval = param_integer( "CKPT_SERVER_INTERVAL",RECLAIM_INTERVAL );

	collection_log = param( "CKPT_SERVER_CLASSAD_FILE" );
	if ( collection_log ) {
		delete CkptClassAds;
		CkptClassAds = new ClassAdCollection(collection_log);
		free(collection_log);
	} else {
		delete CkptClassAds;
		CkptClassAds = NULL;
	}

	clean_interval = param_integer( "CKPT_SERVER_CLEAN_INTERVAL",CLEAN_INTERVAL );
	check_parent_interval = param_integer( "CKPT_SERVER_CHECK_PARENT_INTERVAL",120);
	socket_bufsize = param_integer( "CKPT_SERVER_SOCKET_BUFSIZE",0 );
	max_xfers = param_integer( "CKPT_SERVER_MAX_PROCESSES",DEFAULT_XFERS );
	max_store_xfers = param_integer( "CKPT_SERVER_MAX_STORE_PROCESSES",max_xfers );
	max_restore_xfers = param_integer( "CKPT_SERVER_MAX_RESTORE_PROCESSES",max_xfers );

	max_replicate_xfers = max_store_xfers/5;

#ifdef WANT_NETMAN
	char		*tmp;				//declare this down here
									//so the compiler doesn't argue
	tmp = param( "MANAGE_BANDWIDTH" );
	if (!tmp) {
		ManageBandwidth = false;
	} else {
		if (tmp[0] == 'T' || tmp[0] == 't') {
			ManageBandwidth = true;
		} else {
			ManageBandwidth = false;
		}
		free(tmp);
		NetworkHorizon = param_integer( "NETWORK_HORIZON",300 );
	}
#endif

	if (first_time) {
		store_req_sd = SetUpPort(CKPT_SVR_STORE_REQ_PORT);
		restore_req_sd = SetUpPort(CKPT_SVR_RESTORE_REQ_PORT);
		service_req_sd = SetUpPort(CKPT_SVR_SERVICE_REQ_PORT);
		replicate_req_sd = SetUpPort(CKPT_SVR_REPLICATE_REQ_PORT);
		max_req_sd_plus1 = store_req_sd;
		if (restore_req_sd > max_req_sd_plus1)
			max_req_sd_plus1 = restore_req_sd;
		if (service_req_sd > max_req_sd_plus1)
			max_req_sd_plus1 = service_req_sd;
		if (replicate_req_sd > max_req_sd_plus1)
			max_req_sd_plus1 = replicate_req_sd;
		max_req_sd_plus1++;
		InstallSigHandlers();
	}
	SetUpPeers();
	xfer_summary.init();
	Log("Server Initializing");
	Log("Server:                            ");
	if (condor_gethostname(hostname, 99) == 0) {
		hostname[99] = '\0';
		Log(hostname);
    }
	else
		Log("Cannot resolve host name");
	sprintf(log_msg, "%s%d", "Store Request Port:                ", 
			CKPT_SVR_STORE_REQ_PORT);
	Log(log_msg);
	sprintf(log_msg, "%s%d", "Store Request Socket Descriptor:   ",
			store_req_sd);
	Log(log_msg);
	len = sizeof(buf_size);
	if (getsockopt(store_req_sd, SOL_SOCKET, SO_RCVBUF, (char*) &buf_size,
				   &len) < 0)
		sprintf(log_msg, "%s%s", "Store Request Buffer Size:         ",
				"Cannot access buffer size");
	else
		sprintf(log_msg, "%s%d", "Store Request Buffer Size:         ", 
				buf_size);
	Log(log_msg);
	sprintf(log_msg, "%s%d", "Restore Request Port:              ",
			CKPT_SVR_RESTORE_REQ_PORT);
	Log(log_msg);
	sprintf(log_msg, "%s%d", "Restore Request Socket Descriptor: ",
			restore_req_sd);
	Log(log_msg);
	len = sizeof(buf_size);
	if (getsockopt(restore_req_sd, SOL_SOCKET, SO_RCVBUF, (char*) &buf_size,
				   &len) < 0)
		sprintf(log_msg, "%s%s", "Restore Request Buffer Size:       ",
				"Cannot access buffer size");
	else
		sprintf(log_msg, "%s%d", "Restore Request Buffer Size:       ", buf_size);
	Log(log_msg);
	sprintf(log_msg, "%s%d", "Service Request Port:              " ,
			CKPT_SVR_SERVICE_REQ_PORT);
	Log(log_msg);
	sprintf(log_msg, "%s%d", "Service Request Socket Descriptor: ",
			service_req_sd);
	Log(log_msg);
	len = sizeof(buf_size);
	if (getsockopt(service_req_sd, SOL_SOCKET, SO_RCVBUF, (char*) &buf_size,
				   &len) < 0)
		sprintf(log_msg, "%s%s", "Service Request Buffer Size:       ", 
				"Cannot access buffer size");
	else
		sprintf(log_msg, "%s%d", "Service Request Buffer Size:       ",
				buf_size);
	Log(log_msg);
	Log("Signal handlers installed:         SIGCHLD");
	Log("                                   SIGUSR1");
	Log("                                   SIGUSR2");
	Log("                                   SIGALRM");
	sprintf(log_msg, "%s%d", "Total allowable transfers:         ", max_xfers);
	Log(log_msg);
	sprintf(log_msg, "%s%d", "Number of storing transfers:       ",
			max_store_xfers);
	Log(log_msg);
	sprintf(log_msg, "%s%d", "Number of restoring transfers:     ",
			max_restore_xfers);
	Log(log_msg);

	first_time = false;
}


int Server::SetUpPort(u_short port)
{
  struct sockaddr_in socket_addr;
  int                temp_sd;
  int                ret_code;

  temp_sd = I_socket();
  if (temp_sd == INSUFFICIENT_RESOURCES) {
      dprintf(D_ALWAYS, "ERROR: insufficient resources for new socket\n");
      exit(INSUFFICIENT_RESOURCES);
    }
  else if (temp_sd == CKPT_SERVER_SOCKET_ERROR) {
      dprintf(D_ALWAYS, "ERROR: cannot open a server request socket\n");
      exit(CKPT_SERVER_SOCKET_ERROR);
  }
  memset((char*) &socket_addr, 0, sizeof(struct sockaddr_in));
  socket_addr.sin_family = AF_INET;
  socket_addr.sin_port = htons(port);
  // Let OS choose address --Sonny 5/18/2005
  //memcpy((char*) &socket_addr.sin_addr, (char*) &server_addr, 
  //	 sizeof(struct in_addr));
  if ((ret_code=I_bind(temp_sd, &socket_addr, TRUE)) != CKPT_OK) {
      dprintf(D_ALWAYS, "ERROR: I_bind() returned an error (#%d)\n", ret_code);
      exit(ret_code);
  }
  if (I_listen(temp_sd, 5) != CKPT_OK) {
      dprintf(D_ALWAYS, "ERROR: I_listen() failed\n");
      exit(LISTEN_ERROR);
  }
  if (ntohs(socket_addr.sin_port) != port) {
      dprintf(D_ALWAYS, "ERROR: cannot use Condor well-known port\n");
      exit(BIND_ERROR);
  }
  // Now we can set server_addr with the address that temp_sd is bound to
  struct sockaddr_in *tmp = getSockAddr(temp_sd);
  if (tmp == NULL) {
      dprintf(D_ALWAYS, "failed getSockAddr(%d)\n", temp_sd);
      exit(BIND_ERROR);
  }
  memcpy( (char*) &server_addr, (char*) &tmp->sin_addr.s_addr, 
		  sizeof(struct in_addr) );

  return temp_sd;
}

void Server::SetUpPeers()
{
	char *peers, *peer, peer_addr[256], *ckpt_host;
	StringList peer_name_list;

	if ((peers = param("CKPT_SERVER_HOSTS")) == NULL) {
		return;
	}

	if( (ckpt_host = param("CKPT_SERVER_HOST")) == NULL ) {
		return;
	}

	peer_name_list.initializeFromString(peers);
	free( peers );
	peer_name_list.rewind();
	while ((peer = peer_name_list.next()) != NULL) {
		if( strcmp(peer, ckpt_host) ) {
			sprintf(peer_addr, "<%s:%d>", peer, CKPT_SVR_REPLICATE_REQ_PORT);
			string_to_sin(peer_addr, peer_addr_list+(num_peers++));
		}
	}
	free( ckpt_host );
}

void Server::Execute()
{
	fd_set         req_sds;
	int            num_sds_ready;
	time_t         current_time;
	time_t         last_reclaim_time;
	time_t		   last_clean_time;
	time_t         last_check_parent_time;
	struct timeval poll;
	int            ppid;
	
	current_time = last_reclaim_time = last_clean_time = last_check_parent_time = time(NULL);

	ppid = getppid();

	dprintf( D_ALWAYS, "Sending initial ckpt server ad to collector\n" );
    struct sockaddr_in *tmp = getSockAddr(store_req_sd);
    if (tmp == NULL) {
        EXCEPT("Can't get the address that store_req_sd is bound to");
    }
    char *canon_name = strdup( inet_ntoa( server_addr) );
	if ( canon_name == NULL ) {
        EXCEPT("strdup() of server address failed");
	}
    xfer_summary.time_out(current_time, canon_name);

	while (more) {                          // Continues until SIGUSR2 signal
		poll.tv_sec = reclaim_interval - ((unsigned int)current_time -
										  (unsigned int)last_reclaim_time);
		poll.tv_usec = 0;

		if( check_parent_interval > 0 ) {
			if( last_check_parent_time > current_time ) {
					// time has jumped back
				last_check_parent_time = current_time;
			}
			if( poll.tv_sec > check_parent_interval -
				(current_time - last_check_parent_time) )
			{
				poll.tv_sec = check_parent_interval;
			}
		}

		errno = 0;
		FD_ZERO(&req_sds);
		FD_SET(store_req_sd, &req_sds);
		FD_SET(restore_req_sd, &req_sds);
		FD_SET(service_req_sd, &req_sds);
		FD_SET(replicate_req_sd, &req_sds);
		// Can reduce the number of calls to the reclaimer by blocking until a
		//   request comes in (change the &poll argument to NULL).  The 
		//   philosophy is that one does not need to reclaim a child process
		//   until another is ready to start
		UnblockSignals();
		while( (more) && 
			   ((num_sds_ready=select(max_req_sd_plus1,
									  (SELECT_FDSET_PTR)&req_sds, 
									  NULL, NULL, &poll)) > 0)) {
		        BlockSignals();
	   
			if (FD_ISSET(service_req_sd, &req_sds))
				HandleRequest(service_req_sd, SERVICE_REQ);
			if (FD_ISSET(store_req_sd, &req_sds))
				HandleRequest(store_req_sd, STORE_REQ);
			if (FD_ISSET(restore_req_sd, &req_sds))
				HandleRequest(restore_req_sd, RESTORE_REQ);
			if (FD_ISSET(replicate_req_sd, &req_sds))
				HandleRequest(replicate_req_sd, REPLICATE_REQ);
			FD_ZERO(&req_sds);
			FD_SET(store_req_sd, &req_sds);
			FD_SET(restore_req_sd, &req_sds);
			FD_SET(service_req_sd, &req_sds);	  
			FD_SET(replicate_req_sd, &req_sds);

			UnblockSignals();
		}
		BlockSignals();
		if (num_sds_ready < 0)
			if (errno == ECHILD) {
				// Note: we shouldn't really get an ECHILD here, but
				// we did (see condor-admin 14075).
				dprintf(D_ALWAYS, "WARNING: got ECHILD from select()\n");
			} else if (errno != EINTR) {
				dprintf(D_ALWAYS, 
						"ERROR: cannot select from request ports, errno = "
						"%d (%s)\n", errno, strerror(errno));
				exit(SELECT_ERROR);
			}
		current_time = time(NULL);
		if (((unsigned int) current_time - (unsigned int) last_reclaim_time) 
				>= (unsigned int) reclaim_interval) {
			transfers.Reclaim(current_time);
			last_reclaim_time = current_time;
			if (replication_level)
				Replicate();
			dprintf(D_ALWAYS, "Sending ckpt server ad to collector...\n"); 
			xfer_summary.time_out(current_time, canon_name);
		}
		if (((unsigned int) current_time - (unsigned int) last_clean_time)
				>= (unsigned int) clean_interval) {
			if (CkptClassAds) {
				Log("Cleaning ClassAd log...");
				CkptClassAds->TruncLog();
				Log("Done cleaning ClassAd log...");
			}
			last_clean_time = current_time;
		}
		if( current_time - last_check_parent_time >= check_parent_interval
			&& check_parent_interval > 0 )
		{
			if( kill(ppid,0) < 0 ) {
				if( errno == ESRCH ) {
					MyString msg;
					msg.sprintf("Parent process %d has gone away", ppid);
					NoMore(msg.Value());
				}
			}
			last_check_parent_time = current_time;
		}
    }
	free( canon_name );
	close(service_req_sd);
	close(store_req_sd);
	close(restore_req_sd);
	close(replicate_req_sd);
}


void Server::HandleRequest(int req_sd,
						   request_type req)
{
	struct sockaddr_in shadow_sa;
	int                shadow_sa_len;
	int                req_len;
	int                new_req_sd;
	service_req_pkt    service_req;
	store_req_pkt      store_req;
	restore_req_pkt    restore_req;
	replicate_req_pkt  replicate_req;
	store_reply_pkt    store_reply;
	restore_reply_pkt  restore_reply;
	char*              buf_ptr;
	int                bytes_recvd=0;
	int                temp_len;
	char               log_msg[256];
	
	shadow_sa_len = sizeof(shadow_sa);
	if ((new_req_sd=I_accept(req_sd, &shadow_sa, &shadow_sa_len)) == 
		ACCEPT_ERROR) {
		dprintf(D_ALWAYS, "I_accept failed.\n");
		exit(ACCEPT_ERROR);
    }
	req_ID++;
	dprintf(D_ALWAYS, "----------------------------------------------------\n");
	switch (req) {
        case SERVICE_REQ:
		    sprintf(log_msg, "%s%s", "Receiving SERVICE request from ", 
					inet_ntoa(shadow_sa.sin_addr));
			break;
		case STORE_REQ:
			sprintf(log_msg, "%s%s", "Receiving STORE request from ", 
					inet_ntoa(shadow_sa.sin_addr));
			break;
		case RESTORE_REQ:
			sprintf(log_msg, "%s%s", "Receiving RESTORE request from ", 
					inet_ntoa(shadow_sa.sin_addr));
			break;
		case REPLICATE_REQ:
			sprintf(log_msg, "%s%s", "Receiving REPLICATE request from ", 
					inet_ntoa(shadow_sa.sin_addr));
			break;
		default:
			dprintf(D_ALWAYS, "ERROR: invalid request type encountered (%d)\n", 
					(int) req);
			exit(BAD_REQUEST_TYPE);
		}
	Log(req_ID, log_msg);
	sprintf(log_msg, "%s%d%s", "Using descriptor ", new_req_sd, 
			" to handle request");
	Log(log_msg);
	if ((req == STORE_REQ) || (req == RESTORE_REQ) || (req == REPLICATE_REQ)) {
		if ((num_store_xfers+num_restore_xfers == max_xfers) ||
			((req == STORE_REQ) && (num_store_xfers == max_store_xfers)) ||
			((req == RESTORE_REQ) &&
			 (num_restore_xfers == max_restore_xfers)) ||
			((req == REPLICATE_REQ) &&
			 (num_replicate_xfers == max_replicate_xfers))) {
			if (req == STORE_REQ || req == REPLICATE_REQ) {
				store_reply.server_name.s_addr = htonl(0);
				store_reply.port = htons(0);
				store_reply.req_status = htons(INSUFFICIENT_BANDWIDTH);
				net_write(new_req_sd, (char*) &store_reply, 
						  sizeof(store_reply_pkt));
				sprintf(log_msg, "%s", 
						"Request to store a file has been DENIED");
			} else {
				restore_reply.server_name.s_addr = htonl(0);
				restore_reply.port = htons(0);
				restore_reply.file_size = htonl(0);
				restore_reply.req_status = htons(INSUFFICIENT_BANDWIDTH);
				net_write(new_req_sd, (char*) &restore_reply, 
						  sizeof(restore_reply_pkt));	      
				sprintf(log_msg, "%s", 
						"Request to restore a file has been DENIED");
			}
			Log(req_ID, log_msg);
			Log("Configured maximum number of active transfers exceeded");
			close(new_req_sd);
			return;
		} 
    }

	switch (req) {
        case SERVICE_REQ:
		    req_len = sizeof(service_req_pkt);
			buf_ptr = (char*) &service_req;
			break;
		case STORE_REQ:
			req_len = sizeof(store_req_pkt);
			buf_ptr = (char*) &store_req;
			break;
		case RESTORE_REQ:
			req_len = sizeof(restore_req_pkt);
			buf_ptr = (char*) &restore_req;
			break;
		case REPLICATE_REQ:
			req_len = sizeof(replicate_req_pkt);
			buf_ptr = (char*) &replicate_req;
			break;
	}

	rt_alarm.SetAlarm(new_req_sd, REQUEST_TIMEOUT);

	while (bytes_recvd < req_len) {
		errno = 0;
		temp_len = read(new_req_sd, buf_ptr+bytes_recvd, req_len-bytes_recvd);
		if (temp_len <= 0) {
			if (errno != EINTR) {
				rt_alarm.ResetAlarm();
				close(new_req_sd);
				sprintf(log_msg,
						"Incomplete request packet from %s [REJECTING]",
						inet_ntoa(shadow_sa.sin_addr));
				Log(req_ID, log_msg);
				sprintf(log_msg,
						"DEBUG: %d bytes recieved; %d bytes expected",
						bytes_recvd, req_len);
				Log(-1, log_msg);
				return;
			}
		} else
			bytes_recvd += temp_len;
    }
	rt_alarm.ResetAlarm();
	switch (req) {
    case SERVICE_REQ:
		ProcessServiceReq(req_ID, new_req_sd, shadow_sa.sin_addr, service_req);
		break;
    case STORE_REQ:
		ProcessStoreReq(req_ID, new_req_sd, shadow_sa.sin_addr, store_req);
		break;
    case RESTORE_REQ:
		ProcessRestoreReq(req_ID, new_req_sd, shadow_sa.sin_addr, restore_req);
		break;
	case REPLICATE_REQ:
		store_req.file_size = replicate_req.file_size;
		store_req.ticket = replicate_req.ticket;
		store_req.priority = replicate_req.priority;
		store_req.time_consumed = replicate_req.time_consumed;
		store_req.key = replicate_req.key;
		strcpy(store_req.filename, replicate_req.filename);
		strcpy(store_req.owner, replicate_req.owner);
		ProcessStoreReq(req_ID, new_req_sd, replicate_req.shadow_IP,
						store_req);
		break;
    }  
	
}


void Server::ProcessServiceReq(int             req_id,
							   int             req_sd,
							   struct in_addr  shadow_IP,
							   service_req_pkt service_req)
{  
	service_reply_pkt  service_reply;
	char               log_msg[256];
	struct sockaddr_in server_sa;
	int                data_conn_sd;
	struct stat        chkpt_file_status;
	char               pathname[MAX_PATHNAME_LENGTH];
	int                num_files;
	int                child_pid;
	int                ret_code;

	service_req.ticket = ntohl(service_req.ticket);
	if (service_req.ticket != AUTHENTICATION_TCKT) {
		service_reply.server_addr.s_addr = htonl(0);
		service_reply.port = htons(0);
		service_reply.req_status = htons(BAD_AUTHENTICATION_TICKET);
		net_write(req_sd, (char*) &service_reply, sizeof(service_reply_pkt));
		sprintf(log_msg, "Request for service from %s DENIED:", 
				inet_ntoa(shadow_IP));
		Log(log_msg);
		Log("Invalid authentication ticket used");
		close(req_sd);
		return;
    }
	service_req.service = ntohs(service_req.service);
	service_req.key = ntohl(service_req.key);

	switch (service_req.service) {
        case CKPT_SERVER_SERVICE_STATUS:
		    sprintf(log_msg, "Service: CKPT_SERVER_SERVICE_STATUS");
			break;
		case SERVICE_RENAME:
			sprintf(log_msg, "Service: SERVICE_RENAME");
			break;
		case SERVICE_DELETE:
			sprintf(log_msg, "Service: SERVICE_DELETE");
			break;
		case SERVICE_EXIST:
			sprintf(log_msg, "Service: SERVICE_EXIST");
			break;
		}
	Log(log_msg);  
	sprintf(log_msg, "Owner:   %s", service_req.owner_name);
	Log(log_msg);  
	sprintf(log_msg, "File:    %s", service_req.file_name);
	Log(log_msg);  

	/* Make sure the various pieces we will be using from the client 
		don't escape the checkpointing directory by rejecting
		it if it is '.' '..' or contains a path separator. */

	if (ValidateNoPathComponents(service_req.owner_name) == FALSE) {
		service_reply.server_addr.s_addr = htonl(0);
		service_reply.port = htons(0);
		service_reply.req_status = htons(BAD_REQ_PKT);
		net_write(req_sd, (char*) &service_reply, sizeof(service_reply_pkt));
		sprintf(log_msg, "Owner field contans illegal path components!");
		Log(log_msg);
		sprintf(log_msg, "Service request DENIED!");
		Log(log_msg);
		close(req_sd);
		return;
	}

	if (ValidateNoPathComponents(service_req.file_name) == FALSE) {
		service_reply.server_addr.s_addr = htonl(0);
		service_reply.port = htons(0);
		service_reply.req_status = htons(BAD_REQ_PKT);
		net_write(req_sd, (char*) &service_reply, sizeof(service_reply_pkt));
		sprintf(log_msg, "Filename field contans illegal path components");
		Log(log_msg);
		sprintf(log_msg, "Service request DENIED!");
		Log(log_msg);
		close(req_sd);
		return;
	}

	if (ValidateNoPathComponents(inet_ntoa(shadow_IP)) == FALSE) {
		service_reply.server_addr.s_addr = htonl(0);
		service_reply.port = htons(0);
		service_reply.req_status = htons(BAD_REQ_PKT);
		net_write(req_sd, (char*) &service_reply, sizeof(service_reply_pkt));
		sprintf(log_msg, "ShadowIpAddr field contans illegal path components");
		Log(log_msg);
		sprintf(log_msg, "Service request DENIED!");
		Log(log_msg);
		close(req_sd);
		return;
	}

	switch (service_req.service) {
        case CKPT_SERVER_SERVICE_STATUS:
		    num_files = imds.GetNumFiles();
			service_reply.num_files = htonl(num_files);
			if (num_files > 0) {
				data_conn_sd = I_socket();
				if (data_conn_sd == INSUFFICIENT_RESOURCES) {
					service_reply.server_addr.s_addr = htonl(0);
					service_reply.port = htons(0);
					service_reply.req_status = 
						htons(abs(INSUFFICIENT_RESOURCES));
					net_write(req_sd, (char*) &service_reply, 
							  sizeof(service_reply));
					sprintf(log_msg, "Service request from %s DENIED:", 
							inet_ntoa(shadow_IP));
					Log(log_msg);
					Log("Insufficient buffers/ports to handle request");
					close(req_sd);
					return;
				} else if (data_conn_sd == CKPT_SERVER_SOCKET_ERROR) {
					service_reply.server_addr.s_addr = htonl(0);
					service_reply.port = htons(0);
					service_reply.req_status = htons(abs(CKPT_SERVER_SOCKET_ERROR));
					net_write(req_sd, (char*) &service_reply, 
							  sizeof(service_reply));
					sprintf(log_msg, "Service request from %s DENIED:", 
							inet_ntoa(shadow_IP));
					Log(log_msg);
					Log("Cannot obtain a new socket from server");
					close(req_sd);
					return;
				}
				memset((char*) &server_sa, 0, sizeof(server_sa));
				server_sa.sin_family = AF_INET;
                // let OS choose address
				//server_sa.sin_addr = server_addr;
				//server_sa.sin_port = htons(0);
				if ((ret_code=I_bind(data_conn_sd,&server_sa,FALSE)) != CKPT_OK)
				{
					dprintf( D_ALWAYS,
							 "ERROR: I_bind() returned an error (#%d)\n", 
							 ret_code );
					exit(ret_code);
				}
				if (I_listen(data_conn_sd, 1) != CKPT_OK) {
					dprintf(D_ALWAYS, "ERROR: I_listen() failed to listen\n");
					exit(LISTEN_ERROR);
				}
				service_reply.server_addr = server_addr;
		  // From the I_bind() call, the port should already be in network-byte
		  //   order
				service_reply.port = server_sa.sin_port;
			} else {
				service_reply.server_addr.s_addr = htonl(0);
				service_reply.port = htons(0);
			}
			service_reply.req_status = htons(CKPT_OK);
			strcpy(service_reply.capacity_free_ACD, "0");
	  		sprintf(log_msg, "STATUS service address: %s:%d", 
	  			inet_ntoa(server_addr), ntohs(service_reply.port));
	  		Log(log_msg);
			break;

		case SERVICE_RENAME:
			service_reply.server_addr.s_addr = htonl(0);
			service_reply.port = htons(0);
			service_reply.num_files = htons(0);
			if ((strlen(service_req.owner_name) == 0) || 
				(strlen(service_req.file_name) == 0) || 
				(strlen(service_req.new_file_name) == 0)) {
				service_reply.req_status = htons(BAD_REQ_PKT);
			} else {
				service_req.key = ntohl(service_req.key);
				if (transfers.GetKey(shadow_IP, service_req.owner_name, 
								 service_req.file_name) == service_req.key) {
					transfers.SetOverride(shadow_IP, 
										  service_req.owner_name, 
										  service_req.file_name);
				}
				if (transfers.GetKey(shadow_IP, service_req.owner_name, 
							 service_req.new_file_name) == service_req.key) {
					transfers.SetOverride(shadow_IP, 
										  service_req.owner_name, 
										  service_req.new_file_name);
				}
				service_reply.req_status = 
					htons(imds.RenameFile(shadow_IP, 
										  service_req.owner_name, 
										  service_req.file_name,
										  service_req.new_file_name));
				if (replication_level) {
					ScheduleReplication(shadow_IP,
										service_req.owner_name,
										service_req.new_file_name,
										replication_level);
				}
			}
			break;

		case SERVICE_COMMIT_REPLICATION:
			service_reply.server_addr.s_addr = htonl(0);
			service_reply.port = htons(0);
			service_reply.num_files = htons(0);
			if ((strlen(service_req.owner_name) == 0) || 
				(strlen(service_req.file_name) == 0) || 
				(strlen(service_req.new_file_name) == 0)) {
				service_reply.req_status = htons(BAD_REQ_PKT);
			} else {
				service_req.key = ntohl(service_req.key);
				if (transfers.GetKey(service_req.shadow_IP,
									 service_req.owner_name, 
								 service_req.file_name) == service_req.key) {
					transfers.SetOverride(service_req.shadow_IP, 
										  service_req.owner_name, 
										  service_req.file_name);
				}
				if (transfers.GetKey(service_req.shadow_IP,
									 service_req.owner_name, 
							 service_req.new_file_name) == service_req.key) {
					transfers.SetOverride(service_req.shadow_IP, 
										  service_req.owner_name, 
										  service_req.new_file_name);
				}
				service_reply.req_status = 
					htons(imds.RenameFile(service_req.shadow_IP, 
										  service_req.owner_name, 
										  service_req.file_name,
										  service_req.new_file_name));
			}
			break;

		case SERVICE_ABORT_REPLICATION:
			shadow_IP = service_req.shadow_IP;
			// no break == fall through to SERVICE_DELETE

		case SERVICE_DELETE:
			service_reply.server_addr.s_addr = htonl(0);
			service_reply.port = htons(0);
			service_reply.num_files = htons(0);
			if ((strlen(service_req.owner_name) == 0) || 
				(strlen(service_req.file_name) == 0))
				service_reply.req_status = htons(BAD_REQ_PKT);
			else {
				service_reply.req_status = 
					htons(imds.RemoveFile(shadow_IP, 
										  service_req.owner_name, 
										  service_req.file_name));
				MyString key;
				key.sprintf("%s/%s/%s", inet_ntoa(shadow_IP), 
						service_req.owner_name, service_req.file_name);
				if (CkptClassAds) {
					CkptClassAds->DestroyClassAd(key.Value());
				}
			}
			break;

		case SERVICE_EXIST:
			service_reply.server_addr.s_addr = htonl(0);
			service_reply.port = htons(0);
			service_reply.num_files = htons(0);
			if ((strlen(service_req.owner_name) == 0) || 
				(strlen(service_req.file_name) == 0))
				service_reply.req_status = htons(BAD_REQ_PKT);
			else {
				sprintf(pathname, "%s%s/%s/%s", LOCAL_DRIVE_PREFIX, 
						inet_ntoa(shadow_IP), service_req.owner_name, 
						service_req.file_name);

				sprintf(log_msg, "Checking existance of file: %s",
					pathname);
				Log(log_msg);

				if (stat(pathname, &chkpt_file_status) == 0) {
						service_reply.req_status = htons(0);
				} else
					service_reply.req_status = htons(DOES_NOT_EXIST);
			}
			break;
			
		default:
			service_reply.server_addr.s_addr = htonl(0);
			service_reply.port = htons(0);
			service_reply.num_files = htons(0);
			service_reply.req_status = htons(BAD_SERVICE_TYPE);
		}
			
	if (net_write(req_sd, (char*) &service_reply, sizeof(service_reply)) < 0) {
		close(req_sd);
		sprintf(log_msg, "Service request from %s DENIED:", 
				inet_ntoa(shadow_IP));
		Log(log_msg);
		Log("Cannot sent IP/port to shadow (socket closed)");
    } else {
		close(req_sd);
		if (service_req.service == CKPT_SERVER_SERVICE_STATUS) {
			if (num_files == 0) {
				Log("Request for server status GRANTED:");
				Log("No files on checkpoint server");
			} else {
				child_pid = fork();
				if (child_pid < 0) {
					close(data_conn_sd);
					Log("Unable to honor status service request:");
					Log("Cannot fork child processes");	  
				} else if (child_pid != 0)  {
					// Parent (Master) process
					transfers.Insert(child_pid, req_id, shadow_IP,
									 "Server Status",
									 "Condor", 0, service_req.key, 0,
									 FILE_STATUS);
					Log("Request for server status GRANTED");
				} else {
					// Child process
					close(store_req_sd);
					close(restore_req_sd);
					close(service_req_sd);
					SendStatus(data_conn_sd);
					exit(CHILDTERM_SUCCESS);
				}
			}
		} else if (service_reply.req_status == htons(CKPT_OK))
			Log("Service request successfully completed");
		else {
			Log("Service request cannot be completed:");
			sprintf(log_msg, "Attempt returns error #%d", 
					ntohs(service_reply.req_status));
			Log(log_msg);
		}
	}
}


void Server::ScheduleReplication(struct in_addr shadow_IP, char *owner,
								 char *filename, int level)
{
	char        log_msg[256];
	static bool first_time = true;

	if (first_time) {
		first_time = false;
		srandom(time(NULL));
	}

	int first_peer = (int)(random()%(long)num_peers);
	for (int i=0; i < level && i < num_peers; i++) {
		ReplicationEvent *e = new ReplicationEvent();
		e->Prio(i);
		e->ServerAddr(peer_addr_list[(first_peer+i)%num_peers]);
		e->ShadowIP(shadow_IP);
		e->Owner(owner);
		e->File(filename);
		sprintf(log_msg, "Scheduling Replication: Prio=%d, Serv=%s, File=%s",
				i, sin_to_string(peer_addr_list+((first_peer+i)%num_peers)),
				filename);
		Log(log_msg);
		replication_schedule.InsertReplicationEvent(e);
	}
}

void Server::Replicate()
{
	int 				child_pid, bytes_recvd=0, bytes_read;
	int					bytes_sent=0, bytes_written, bytes_left;
	char        		log_msg[256], buf[10240];
	char				pathname[MAX_PATHNAME_LENGTH];
	int 				server_sd, ret_code, fd;
	struct sockaddr_in 	server_sa;
	struct stat 		chkpt_file_status;
	time_t				file_timestamp;
	replicate_req_pkt 	req;
	replicate_reply_pkt	reply;

	Log("Checking replication schedule...");
	ReplicationEvent *e = replication_schedule.GetNextReplicationEvent();
	if (e) {
		server_sa = e->ServerAddr();
		sprintf(log_msg, "Replicating: Prio=%d, Serv=%s, File=%s",
				e->Prio(), sin_to_string(&server_sa), e->File());
		Log(log_msg);
		sprintf(pathname, "%s%s/%s/%s", LOCAL_DRIVE_PREFIX,
				inet_ntoa(e->ShadowIP()), e->Owner(), e->File());
		if (stat(pathname, &chkpt_file_status) < 0) {
			dprintf(D_ALWAYS, "ERROR: File '%s' can not be accessed.\n", 
					pathname);
			exit(DOES_NOT_EXIST);
		}
		req.file_size = htonl(chkpt_file_status.st_size);
#undef AVOID_FORK
#if !defined(AVOID_FORK)
		child_pid = fork();
		if (child_pid < 0) {
			Log("Unable to perform replication.  Cannot fork child process.");
		} else if (child_pid == 0) {
#endif
			req.ticket = htonl(AUTHENTICATION_TCKT);
			req.priority = htonl(0);
			req.time_consumed = htonl(0);
			req.key = htonl(getpid()); // from server_interface.c
			strcpy(req.owner, e->Owner());
//			strcpy(req.filename, e->File());
			sprintf(req.filename, "%s.rep", e->File());
			req.shadow_IP = e->ShadowIP();
			file_timestamp = chkpt_file_status.st_mtime;
			if ((server_sd = I_socket()) < 0) {
				dprintf(D_ALWAYS, "ERROR: I_socket failed.\n");
				exit(server_sd);
			}
			/* TRUE means this is an outgoing connection */
			if( ! _condor_local_bind(TRUE, server_sd) ) {
				close( server_sd );
				dprintf(D_ALWAYS, "ERROR: unable to bind new socket to local interface\n");
				exit(1);
			}
			ret_code = net_write(server_sd, (char *)&req, sizeof(req));
			if (ret_code != sizeof(req)) {
				dprintf(D_ALWAYS, 
						"ERROR: write failed, wrote %d bytes instead of %d bytes\n", 
						ret_code, (int)sizeof(req));
				exit(CHILDTERM_CANNOT_WRITE);
			}
			if (connect(server_sd, (struct sockaddr*) &server_sa,
						sizeof(server_sa)) < 0) {
				dprintf(D_ALWAYS, "ERROR: connect failed.\n");
				exit(CONNECT_ERROR);
			}
			if (net_write(server_sd, (char *) &req,
						  sizeof(req)) != sizeof(req)) {
				dprintf(D_ALWAYS, "ERROR: Write failed\n");
				exit(CHILDTERM_CANNOT_WRITE);
			}
			while (bytes_recvd != sizeof(reply)) {
				errno = 0;
				bytes_read = read(server_sd, ((char *) &reply)+bytes_recvd,
								  sizeof(reply)-bytes_recvd);
				assert(bytes_read >= 0);
				if (bytes_read == 0)
					assert(errno == EINTR);
				else
					bytes_recvd += bytes_read;
			}
			close(server_sd);
			if ((server_sd = I_socket()) < 0) {
				dprintf(D_ALWAYS, "ERROR: I_socket failed (line %d)\n",
						__LINE__);
				exit(server_sd);
			}
			/* TRUE means this is an outgoing connection */
			if( ! _condor_local_bind(TRUE, server_sd) ) {
				close( server_sd );
				dprintf(D_ALWAYS, 
						"ERROR: unable to bind new socket to local interface\n");
				exit(1);
			}
			memset((char*) &server_sa, 0, sizeof(server_sa));
			server_sa.sin_family = AF_INET;
			memcpy((char *) &server_sa.sin_addr.s_addr,
				   (char *) &reply.server_name, sizeof(reply.server_name));
			server_sa.sin_port = reply.port;
			if (connect(server_sd, (struct sockaddr*) &server_sa,
						sizeof(server_sa)) < 0) {
				dprintf(D_ALWAYS, "ERROR: Connect failed (line %d)\n",
						__LINE__);
				exit(CONNECT_ERROR);
			}
			if ((fd = safe_open_wrapper(pathname, O_RDONLY)) < 0) {
				dprintf(D_ALWAYS, "ERROR: Can't open file '%s'\n", pathname);
				exit(CHILDTERM_CANNOT_OPEN_CHKPT_FILE);
			}
			bytes_read = 1;
			while (bytes_sent != req.file_size && bytes_read > 0) {
				errno = 0;
				bytes_read = read(fd, &buf, sizeof(buf));
				bytes_left = bytes_read;
				while (bytes_left) {
					bytes_written = write(server_sd, &buf, bytes_read);
					assert(bytes_written >= 0);
					if (bytes_written == 0)
						assert(errno == EINTR);
					else {
						bytes_sent += bytes_written;
						bytes_left -= bytes_written;
					}
				}
				sleep(1);		// slow pipe
			}
			close(server_sd);
			close(fd);
			chkpt_file_status.st_mtime = 0;
			stat(pathname, &chkpt_file_status);
			if ((server_sd = I_socket()) < 0) {
				dprintf(D_ALWAYS, "ERROR: I_socket failed (line %d)\n", 
						__LINE__);
				exit(server_sd);
			}
			/* TRUE means this is an outgoing connection */
			if( ! _condor_local_bind(TRUE, server_sd) ) {
				close( server_sd );
				dprintf(D_ALWAYS, "ERROR: unable to bind new socket to local interface\n");
				exit(1);
			}
			server_sa.sin_port = htons(CKPT_SVR_SERVICE_REQ_PORT);
			if (connect(server_sd, (struct sockaddr*) &server_sa,
						sizeof(server_sa)) < 0) {
				dprintf(D_ALWAYS, "ERROR: Connect failed (line %d)\n",
						__LINE__);
				exit(CONNECT_ERROR);
			}
			service_req_pkt s_req;
			service_reply_pkt s_rep;
			s_req.shadow_IP = e->ShadowIP();
			s_req.ticket = htonl(AUTHENTICATION_TCKT);
			s_req.key	= htonl(getpid());
			strcpy(s_req.owner_name, e->Owner());
			sprintf(s_req.file_name, "%s.rep", e->File());
			sprintf(s_req.new_file_name, "%s", e->File());
			if (chkpt_file_status.st_mtime == file_timestamp) {
				s_req.service = htons(SERVICE_COMMIT_REPLICATION);
			}
			else {
				s_req.service = htons(SERVICE_ABORT_REPLICATION);
			}
			ret_code = net_write(server_sd, (char *)&s_req, sizeof(s_req));
			if (ret_code != sizeof(s_req)) {
				dprintf(D_ALWAYS, "ERROR: Write failed (%d bytes instead of %d)\n",
						ret_code, (int)sizeof(s_req));
				exit(CHILDTERM_CANNOT_WRITE);
			}
			bytes_recvd = 0;
			while (bytes_recvd != sizeof(s_rep)) {
				errno = 0;
				bytes_read = read(server_sd, ((char *) &s_rep)+bytes_recvd,
								  sizeof(s_rep)-bytes_recvd);
				assert(bytes_read >= 0);
				if (bytes_read == 0)
					assert(errno == EINTR);
				else
					bytes_recvd += bytes_read;
			}
#if !defined(AVOID_FORK)
			exit(CHILDTERM_SUCCESS);
		}
		transfers.Insert(child_pid, -1, e->ShadowIP(), e->File(), e->Owner(),
						 req.file_size, getpid(), 0, REPLICATE);
#endif
		num_replicate_xfers++;
		delete e;
	}
}

void Server::SendStatus(int data_conn_sd)
{
  struct sockaddr_in chkpt_addr;
  int                chkpt_addr_len;
  int                xfer_sd;

  chkpt_addr_len = sizeof(struct sockaddr_in);
  if ((xfer_sd=I_accept(data_conn_sd, &chkpt_addr, &chkpt_addr_len)) ==
      ACCEPT_ERROR)
    {
	  dprintf(D_ALWAYS, "ERROR: I_accept failed.\n");
      exit(CHILDTERM_ACCEPT_ERROR);
    }
  close(data_conn_sd);
  if (socket_bufsize) {
		  // Changing buffer size may fail
	  setsockopt(xfer_sd, SOL_SOCKET, SO_SNDBUF, (char*) &socket_bufsize, 
				 sizeof(socket_bufsize));
  }
  imds.TransferFileInfo(xfer_sd);
}

#ifdef WANT_NETMAN
static struct _file_stream_info {
	struct in_addr shadow_IP;
	int shadow_pid;
	struct in_addr startd_IP;
	bool store_req;
	int total_bytes;
	time_t last_update;
	char user[MAX_NAME_LENGTH];
} file_stream_info;

extern "C" {
void
file_stream_progress_report(int bytes_moved)
{
	if (!ManageBandwidth) return;

	time_t current_time = time(0);
	if (current_time < file_stream_info.last_update + (NetworkHorizon/5))
		return;

	dprintf(D_ALWAYS, "Sending CkptServerUpdate: %d bytes to go.\n", 
			file_stream_info.total_bytes-bytes_moved);
	ClassAd request;
	char buf[100];

	sprintf(buf, "%s = \"CkptServerUpdate\"", ATTR_TRANSFER_TYPE);
	request.Insert(buf);
	sprintf(buf, "%s = \"%s\"", ATTR_USER, file_stream_info.user);
	request.Insert(buf);
	sprintf(buf, "%s = 1", ATTR_FORCE);
	request.Insert(buf);
	sprintf(buf, "%s = \"%s\"", ATTR_SOURCE,
			file_stream_info.store_req ?
			inet_ntoa(file_stream_info.startd_IP) :
			inet_ntoa(*my_sin_addr()));
	request.Insert(buf);
	sprintf(buf, "%s = \"%s\"", ATTR_DESTINATION,
			file_stream_info.store_req ?
			inet_ntoa(*my_sin_addr()) :
			inet_ntoa(file_stream_info.startd_IP));
	request.Insert(buf);
	sprintf(buf, "%s = \"<%s:0>\"", ATTR_REMOTE_HOST,
			inet_ntoa(file_stream_info.startd_IP));
	request.Insert(buf);
	sprintf(buf, "%s = %d", ATTR_REQUESTED_CAPACITY,
			file_stream_info.total_bytes-bytes_moved);
	request.Insert(buf);
	sprintf(buf, "%s = \"%u.%d\"", ATTR_TRANSFER_KEY,
			ntohl(file_stream_info.shadow_IP.s_addr),
			file_stream_info.shadow_pid);
 	request.Insert(buf);
	Daemon Negotiator(DT_NEGOTIATOR);

    SafeSock sock;
    sock.timeout(10);
    if (!sock.connect(Negotiator.addr())) {
		dprintf(D_ALWAYS, "Couldn't connect to negotiator!\n");
    }

	Negotiator.startCommand( REQUEST_NETWORK, &sock);

	sock.put(1);
	request.put(sock);
	sock.end_of_message();
	file_stream_info.last_update = time(0);
}
}
#endif

/* check to make sure something being used as a filename that we will
	be reading or writing isn't trying to do anything funny. This
	means the filename can't be "." ".." or have a path separator
	in it. */
int ValidateNoPathComponents(char *path)
{
	if (path == NULL) {
		return FALSE;
	}

	if (path[0] == '\0' ||
		strcmp(path, ".") == MATCH || 
		strcmp(path, "..") == MATCH ||
		strstr(path, "/") != NULL)
	{
		return FALSE;
	}

	return TRUE;
}

void Server::ProcessStoreReq(int            req_id,
							 int            req_sd,
							 struct in_addr shadow_IP,
							 store_req_pkt  store_req)
{
	int                ret_code;
	store_reply_pkt    store_reply;
	int                data_conn_sd;
	struct sockaddr_in server_sa;
	int                child_pid;
	char               pathname[MAX_PATHNAME_LENGTH];
	char               log_msg[256];
	int                err_code;
	
	store_req.ticket = ntohl(store_req.ticket);
	if (store_req.ticket != AUTHENTICATION_TCKT) {
		store_reply.server_name.s_addr = htonl(0);
		store_reply.port = htons(0);
		store_reply.req_status = htons(BAD_AUTHENTICATION_TICKET);
		net_write(req_sd, (char*) &store_reply, sizeof(store_reply_pkt));
		sprintf(log_msg, "Store request from %s DENIED:", 
				inet_ntoa(shadow_IP));
		Log(log_msg);
		Log("Invalid authentication ticket used");
		close(req_sd);
		return;
    }
	if ((strlen(store_req.filename) == 0) || (strlen(store_req.owner) == 0)) {
		store_reply.server_name.s_addr = htonl(0);
		store_reply.port = htons(0);
		store_reply.req_status = htons(BAD_REQ_PKT);
		net_write(req_sd, (char*) &store_reply, sizeof(store_reply_pkt));
		sprintf(log_msg, "Store request from %s DENIED:", 
				inet_ntoa(shadow_IP));
		Log(log_msg);
		Log("Incomplete request packet");
		close(req_sd);
		return;
    }

	sprintf(log_msg, "Owner:     %s", store_req.owner);
	Log(log_msg);
	sprintf(log_msg, "File name: %s", store_req.filename);
	Log(log_msg);
	sprintf(log_msg, "File size: %d", ntohl(store_req.file_size));
	Log(log_msg);

	/* Make sure the various pieces we will be using from the client 
		don't escape the checkpointing directory by rejecting
		it if it is '.' '..' or contains a path separator. */
	
	if (ValidateNoPathComponents(store_req.owner) == FALSE) {
		store_reply.server_name.s_addr = htonl(0);
		store_reply.port = htons(0);
		store_reply.req_status = htons(BAD_REQ_PKT);
		net_write(req_sd, (char*) &store_reply, sizeof(store_reply_pkt));
		sprintf(log_msg, "Owner field contans illegal path components!");
		Log(log_msg);
		sprintf(log_msg, "STORE request DENIED!");
		Log(log_msg);
		close(req_sd);
		return;
	}

	if (ValidateNoPathComponents(store_req.filename) == FALSE) {
		store_reply.server_name.s_addr = htonl(0);
		store_reply.port = htons(0);
		store_reply.req_status = htons(BAD_REQ_PKT);
		net_write(req_sd, (char*) &store_reply, sizeof(store_reply_pkt));
		sprintf(log_msg, "Filename field contans illegal path components!");
		Log(log_msg);
		sprintf(log_msg, "STORE request DENIED!");
		Log(log_msg);
		close(req_sd);
		return;
	}

	if (ValidateNoPathComponents(inet_ntoa(shadow_IP)) == FALSE) {
		store_reply.server_name.s_addr = htonl(0);
		store_reply.port = htons(0);
		store_reply.req_status = htons(BAD_REQ_PKT);
		net_write(req_sd, (char*) &store_reply, sizeof(store_reply_pkt));
		sprintf(log_msg, "ShadowIpAddr field contans illegal path components!");
		Log(log_msg);
		sprintf(log_msg, "STORE request DENIED!");
		Log(log_msg);
		close(req_sd);
		return;
	}


	store_req.file_size = ntohl(store_req.file_size);
	data_conn_sd = I_socket();
	if (data_conn_sd == INSUFFICIENT_RESOURCES) {
		store_reply.server_name.s_addr = htonl(0);
		store_reply.port = htons(0);
		store_reply.req_status = htons(abs(INSUFFICIENT_RESOURCES));
		net_write(req_sd, (char*) &store_reply, sizeof(store_reply_pkt));
		sprintf(log_msg, "Store request from %s DENIED:", 
				inet_ntoa(shadow_IP));
		Log(log_msg);
		Log("Insufficient buffers/ports to handle request");
		close(req_sd);
		return;
	} else if (data_conn_sd == CKPT_SERVER_SOCKET_ERROR) {
		store_reply.server_name.s_addr = htonl(0);
		store_reply.port = htons(0);
		store_reply.req_status = htons(abs(CKPT_SERVER_SOCKET_ERROR));
		net_write(req_sd, (char*) &store_reply, sizeof(store_reply_pkt));
		sprintf(log_msg, "Store request from %s DENIED:", 
				inet_ntoa(shadow_IP));
		Log(log_msg);
		Log("Cannot obtain a new socket from server");
		close(req_sd);
		return;
	}

	memset((char*) &server_sa, 0, sizeof(server_sa));
	server_sa.sin_family = AF_INET;
    // Let OS choose address
	//server_sa.sin_port = htons(0);
	//server_sa.sin_addr = server_addr;
	if ((err_code=I_bind(data_conn_sd, &server_sa,FALSE)) != CKPT_OK) {
		sprintf(log_msg, "ERROR: I_bind() returns an error (#%d)", 
				err_code);
		Log(0, log_msg);
		exit(ret_code);
	}

	if (I_listen(data_conn_sd, 1) != CKPT_OK) {
		sprintf(log_msg, "ERROR: I_listen() fails to listen");
		Log(log_msg);
		exit(LISTEN_ERROR);
	}

	imds.AddFile(shadow_IP, store_req.owner, store_req.filename, 
				 store_req.file_size, NOT_PRESENT);
	store_reply.server_name = server_addr;
	// From the I_bind() call, the port should already be in network-byte
		   //   order
		   store_reply.port = server_sa.sin_port;  
	store_reply.req_status = htons(CKPT_OK);
	sprintf(log_msg, "STORE service address: %s:%d", 
		inet_ntoa(server_addr), ntohs(store_reply.port));
	Log(log_msg);
	if (net_write(req_sd, (char*) &store_reply, 
				  sizeof(store_reply_pkt)) < 0) {
		close(req_sd);
		sprintf(log_msg, "Store request from %s DENIED:", 
				inet_ntoa(shadow_IP));
		Log(log_msg);
		Log("Cannot send IP/port to shadow (socket closed)");
		imds.RemoveFile(shadow_IP, store_req.owner, store_req.filename);
		close(data_conn_sd);
	} else {
		close(req_sd);

		child_pid = fork();

		if (child_pid < 0) {
			close(data_conn_sd);
			Log("Unable to honor store request:");
			Log("Cannot fork child processes");
		} else if (child_pid != 0) {
			// Parent (Master) process
				   close(data_conn_sd);
			num_store_xfers++;
			transfers.Insert(child_pid, req_id, shadow_IP, 
							 store_req.filename, store_req.owner,
							 store_req.file_size, store_req.key,
							 store_req.priority, RECV);
			Log("Request to store checkpoint file GRANTED");
			int len = strlen(store_req.filename);
			if (strcmp(store_req.filename+len-4, ".tmp") == MATCH) {
				store_req.filename[len-4] = '\0';
			}
			MyString keybuf;
			keybuf.sprintf( "%s/%s/%s", inet_ntoa(shadow_IP), store_req.owner,
					store_req.filename);
			char const *key = keybuf.Value();
			ClassAd *ad;
			if (CkptClassAds) {
				if (!CkptClassAds->LookupClassAd(key, ad)) {
					MyString buf;
					CkptClassAds->NewClassAd(key, CKPT_FILE_ADTYPE, "0");
					buf.sprintf( "\"%s\"", store_req.owner);
					CkptClassAds->SetAttribute(key, ATTR_OWNER, buf.Value());
					buf.sprintf( "\"%s\"", inet_ntoa(shadow_IP));
					CkptClassAds->SetAttribute(key, ATTR_SHADOW_IP_ADDR, buf.Value());
					buf.sprintf( "\"%s\"", store_req.filename);
					CkptClassAds->SetAttribute(key, ATTR_FILE_NAME, buf.Value());
				}
				char size[40];
				sprintf(size, "%d", (int) store_req.file_size);
				CkptClassAds->SetAttribute(key, ATTR_FILE_SIZE, size);
			}
		} else {
			// Child process
			close(store_req_sd);
			close(restore_req_sd);
			close(service_req_sd);
			sprintf(pathname, "%s%s/%s/%s", LOCAL_DRIVE_PREFIX, 
					inet_ntoa(shadow_IP), store_req.owner, 
					store_req.filename);
#ifdef WANT_NETMAN
			memcpy(&file_stream_info.shadow_IP, &shadow_IP,
				   sizeof(struct in_addr));
			file_stream_info.shadow_pid = ntohl(store_req.key);
			file_stream_info.store_req = true;
			file_stream_info.total_bytes = (int) store_req.file_size;
			file_stream_info.last_update = time(0);
			strcpy(file_stream_info.user, store_req.owner);
#endif
			ReceiveCheckpointFile(data_conn_sd, pathname,
								  (int) store_req.file_size);
			exit(CHILDTERM_SUCCESS);
		}
	}
}

void Server::ReceiveCheckpointFile(int         data_conn_sd,
								   const char* pathname,
								   int         file_size)
{
	struct sockaddr_in chkpt_addr;
	int                chkpt_addr_len;
	int                xfer_sd;
	int				   bytes_recvd=0;
	int				   file_fd;
	int				   peer_info_fd;
	char			   peer_info_filename[100];
	bool			   incomplete_file = false;
	char               log_msg[256];
	
	sprintf(log_msg, "Receiving checkpoint to file: %s", pathname);
	Log(log_msg);
	
#if 1
	file_fd = safe_open_wrapper(pathname, O_WRONLY|O_CREAT|O_TRUNC,0664);
#else
	file_fd = safe_open_wrapper("/dev/null", O_WRONLY, 0);
#endif

	if (file_fd < 0) {
		exit(CHILDTERM_CANNOT_OPEN_CHKPT_FILE);
	}
	chkpt_addr_len = sizeof(struct sockaddr_in);
	// May cause an ACCEPT_ERROR error
	do {
		xfer_sd = tcp_accept_timeout(data_conn_sd,
									 (struct sockaddr *)&chkpt_addr, 
									 &chkpt_addr_len, CKPT_ACCEPT_TIMEOUT);
	} while (xfer_sd == -3 || (xfer_sd == -1 && errno == EINTR));
#if 0
	if ((xfer_sd=I_accept(data_conn_sd, &chkpt_addr, &chkpt_addr_len)) ==
		ACCEPT_ERROR) {
#else
	if (xfer_sd < 0) {
#endif
		dprintf(D_ALWAYS, "ERROR: I_accept() failed (line %d)\n",
				__LINE__);
		exit(CHILDTERM_ACCEPT_ERROR);
    }
	close(data_conn_sd);
	if (socket_bufsize) {
			// Changing buffer size may fail
		setsockopt(xfer_sd, SOL_SOCKET, SO_RCVBUF, (char*) &socket_bufsize, 
				   sizeof(socket_bufsize));
	}
#ifdef WANT_NETMAN
	file_stream_info.startd_IP = chkpt_addr.sin_addr;
#endif
	bytes_recvd = stream_file_xfer(xfer_sd, file_fd, file_size);
	// note that if file size == -1, we don't know if we got the complete 
	// file, so we must rely on the client to commit via SERVICE_RENAME
	if (bytes_recvd < file_size) incomplete_file = true;
	int n = htonl(bytes_recvd);
	net_write(xfer_sd, (char*) &n, sizeof(n));
	close(xfer_sd);
	fsync(file_fd);
	close(file_fd);

	// Write peer address and bytes received to a temporary file which
	// is read by the parent.  The peer address is very important for
	// logging purposes.
	sprintf(peer_info_filename, "/tmp/condor_ckpt_server.%d", getpid());
	peer_info_fd = safe_open_wrapper(peer_info_filename, O_WRONLY|O_CREAT|O_TRUNC, 0664);
	if (peer_info_fd >= 0) {
		write(peer_info_fd, (char *)&(chkpt_addr.sin_addr),
			  sizeof(struct in_addr));
		write(peer_info_fd, (char *)&bytes_recvd, sizeof(bytes_recvd));
		close(peer_info_fd);
	}

	if (incomplete_file) {
		dprintf(D_ALWAYS, "ERROR: Incomplete file.\n");
		exit(CHILDTERM_ERROR_INCOMPLETE_FILE);
	}
}


void Server::ProcessRestoreReq(int             req_id,
							   int             req_sd,
							   struct in_addr  shadow_IP,
							   restore_req_pkt restore_req)
{
	struct stat        chkpt_file_status;
	struct sockaddr_in server_sa;
	int                ret_code;
	restore_reply_pkt  restore_reply;
	int                data_conn_sd;
	int                child_pid;
	char               pathname[MAX_PATHNAME_LENGTH];
	int                preexist;
	char               log_msg[256];
	int                err_code;
	
	restore_req.ticket = ntohl(restore_req.ticket);
	if (restore_req.ticket != AUTHENTICATION_TCKT) {
		restore_reply.server_name.s_addr = htonl(0);
		restore_reply.port = htons(0);
		restore_reply.file_size = htonl(0);
		restore_reply.req_status = htons(BAD_AUTHENTICATION_TICKET);
		net_write(req_sd, (char*) &restore_reply, sizeof(restore_reply_pkt));
		sprintf(log_msg, "Restore request from %s DENIED:", 
				inet_ntoa(shadow_IP));
		Log(log_msg);
		Log("Invalid authentication ticket used");
		close(req_sd);
		return;
    }
	if ((strlen(restore_req.filename) == 0) || 
		(strlen(restore_req.owner) == 0)) {
		restore_reply.server_name.s_addr = htonl(0);
		restore_reply.port = htons(0);
		restore_reply.file_size = htonl(0);
		restore_reply.req_status = htons(BAD_REQ_PKT);
		net_write(req_sd, (char*) &restore_reply, sizeof(restore_reply_pkt));
		sprintf(log_msg, "Restore request from %s DENIED:", 
				inet_ntoa(shadow_IP));
		Log(log_msg);
		Log("Incomplete request packet");
		close(req_sd);
		return;
    }

	sprintf(log_msg, "Owner:     %s", restore_req.owner);
	Log(log_msg);
	sprintf(log_msg, "File name: %s", restore_req.filename);
	Log(log_msg);

	/* Make sure the various pieces we will be using from the client 
		don't escape the checkpointing directory by rejecting
		it if it is '.' '..' or contains a path separator. */

	if (ValidateNoPathComponents(restore_req.owner) == FALSE) {
		restore_reply.server_name.s_addr = htonl(0);
		restore_reply.port = htons(0);
		restore_reply.req_status = htons(BAD_REQ_PKT);
		net_write(req_sd, (char*) &restore_reply, sizeof(restore_reply_pkt));
		sprintf(log_msg, "Owner field contans illegal path components!");
		Log(log_msg);
		sprintf(log_msg, "RESTORE request DENIED!");
		Log(log_msg);
		close(req_sd);
		return;
	}

	if (ValidateNoPathComponents(restore_req.filename) == FALSE) {
		restore_reply.server_name.s_addr = htonl(0);
		restore_reply.port = htons(0);
		restore_reply.req_status = htons(BAD_REQ_PKT);
		net_write(req_sd, (char*) &restore_reply, sizeof(restore_reply_pkt));
		sprintf(log_msg, "Filename field contans illegal path components!");
		Log(log_msg);
		sprintf(log_msg, "RESTORE request DENIED!");
		Log(log_msg);
		close(req_sd);
		return;
	}

	if (ValidateNoPathComponents(inet_ntoa(shadow_IP)) == FALSE) {
		restore_reply.server_name.s_addr = htonl(0);
		restore_reply.port = htons(0);
		restore_reply.req_status = htons(BAD_REQ_PKT);
		net_write(req_sd, (char*) &restore_reply, sizeof(restore_reply_pkt));
		sprintf(log_msg, "ShadowIpAddr field contans illegal path components!");
		Log(log_msg);
		sprintf(log_msg, "RESTORE request DENIED!");
		Log(log_msg);
		close(req_sd);
		return;
	}

	sprintf(pathname, "%s%s/%s/%s", LOCAL_DRIVE_PREFIX, inet_ntoa(shadow_IP),
			restore_req.owner, restore_req.filename);
	if ((preexist=stat(pathname, &chkpt_file_status)) != 0) {
		restore_reply.server_name.s_addr = htonl(0);
		restore_reply.port = htons(0);
		restore_reply.file_size = htonl(0);
		restore_reply.req_status = htons(DESIRED_FILE_NOT_FOUND);
		net_write(req_sd, (char*) &restore_reply, sizeof(restore_reply_pkt));
		sprintf(log_msg, "Restore request from %s DENIED:", 
				inet_ntoa(shadow_IP));
		Log(log_msg);
		Log("Requested file does not exist on server");
		close(req_sd);
		return;
    } else if (preexist == 0) {
		imds.AddFile(shadow_IP, restore_req.owner, restore_req.filename, 
					 chkpt_file_status.st_size, ON_SERVER);
	}
      data_conn_sd = I_socket();
      if (data_conn_sd == INSUFFICIENT_RESOURCES) {
		  restore_reply.server_name.s_addr = htonl(0);
		  restore_reply.port = htons(0);
		  restore_reply.file_size = htonl(0);
		  restore_reply.req_status = htons(abs(INSUFFICIENT_RESOURCES));
		  net_write(req_sd, (char*) &restore_reply, sizeof(restore_reply_pkt));
		  sprintf(log_msg, "Restore request from %s DENIED:", 
				  inet_ntoa(shadow_IP));
		  Log(log_msg);
		  Log("Insufficient buffers/ports to handle request");
		  close(req_sd);
		  return;
	  } else if (data_conn_sd == CKPT_SERVER_SOCKET_ERROR) {
		  restore_reply.server_name.s_addr = htonl(0);
		  restore_reply.port = htons(0);
		  restore_reply.file_size = htonl(0);
		  restore_reply.req_status = htons(abs(CKPT_SERVER_SOCKET_ERROR));
		  net_write(req_sd, (char*) &restore_reply, sizeof(restore_reply_pkt));
		  sprintf(log_msg, "Restore request from %s DENIED:", 
				  inet_ntoa(shadow_IP));
		  Log(log_msg);
		  Log("Cannot botain a new socket from server");
		  close(req_sd);
		  return;
	  }
      memset((char*) &server_sa, 0, sizeof(server_sa));
      server_sa.sin_family = AF_INET;
      //server_sa.sin_port = htons(0);
      //server_sa.sin_addr = server_addr;
      if ((err_code=I_bind(data_conn_sd, &server_sa,FALSE)) != CKPT_OK) {
		  sprintf(log_msg, "ERROR: I_bind() returns an error (#%d)", err_code);
		  Log(0, log_msg);
		  exit(ret_code);
	  }
      if (I_listen(data_conn_sd, 1) != CKPT_OK) {
		  sprintf(log_msg, "ERROR: I_listen() fails to listen");
		  Log(0, log_msg);
		  exit(LISTEN_ERROR);
	  }
      restore_reply.server_name = server_addr;
      // From the I_bind() call, the port should already be in network-byte
      //   order
      restore_reply.port = server_sa.sin_port;  
      restore_reply.file_size = htonl(chkpt_file_status.st_size);
      restore_reply.req_status = htons(CKPT_OK);
	  sprintf(log_msg, "RESTORE service address: %s:%d", 
	  	inet_ntoa(server_addr), ntohs(restore_reply.port));
	  Log(log_msg);
      if (net_write(req_sd, (char*) &restore_reply, 
					sizeof(restore_reply_pkt)) < 0) {
		  close(req_sd);
		  sprintf(log_msg, "Restore request from %s DENIED:", 
				  inet_ntoa(shadow_IP));
		  Log(log_msg);
		  Log("Cannot send IP/port to shadow (socket closed)");
		  close(data_conn_sd);
	  } else {
		  close(req_sd);
		  child_pid = fork();
		  if (child_pid < 0) {
			  close(data_conn_sd);
			  Log("Unable to honor restore request!");
			  Log("Cannot fork child processes");
		  } else if (child_pid != 0) {            // Parent (Master) process
			  close(data_conn_sd);
			  num_restore_xfers++;
			  transfers.Insert(child_pid, req_id, shadow_IP, 
							   restore_req.filename, restore_req.owner,
							   chkpt_file_status.st_size, restore_req.key,
							   restore_req.priority, XMIT);
			  Log("Request to restore checkpoint file GRANTED");
		  } else {
			  // Child process
			  close(store_req_sd);
			  close(restore_req_sd);
			  close(service_req_sd);
#ifdef WANT_NETMAN
			  memcpy(&file_stream_info.shadow_IP, &shadow_IP,
					 sizeof(struct in_addr));
			  file_stream_info.shadow_pid = ntohl(restore_req.key);
			  file_stream_info.store_req = false;
			  file_stream_info.total_bytes = (int) chkpt_file_status.st_size;
			  file_stream_info.last_update = time(0);
			  strcpy(file_stream_info.user, restore_req.owner);
#endif
			  TransmitCheckpointFile(data_conn_sd, pathname,
									 chkpt_file_status.st_size);
			  exit(CHILDTERM_SUCCESS);
		  }
	  }
}


void Server::TransmitCheckpointFile(int         data_conn_sd,
									const char* pathname,
									int         file_size)
{
	struct sockaddr_in chkpt_addr;
	int                chkpt_addr_len;
	int                xfer_sd;
	int                bytes_sent=0;
	int				   file_fd;
	int				   peer_info_fd;
	char			   peer_info_filename[100];
	char               log_msg[256];
	
	sprintf(log_msg, "Transmitting checkpoint file: %s", pathname);
	Log(log_msg);

	file_fd = safe_open_wrapper(pathname, O_RDONLY);
	if (file_fd < 0) {
		dprintf(D_ALWAYS, "ERROR: Can't open file '%s'\n", pathname);
		exit(CHILDTERM_CANNOT_OPEN_CHKPT_FILE);
	}

	chkpt_addr_len = sizeof(struct sockaddr_in);
	do {
		xfer_sd = tcp_accept_timeout(data_conn_sd,
									 (struct sockaddr *)&chkpt_addr, 
									 &chkpt_addr_len, CKPT_ACCEPT_TIMEOUT);
	} while (xfer_sd == -3 || (xfer_sd == -1 && errno == EINTR));
#if 0
	if ((xfer_sd=I_accept(data_conn_sd, &chkpt_addr, &chkpt_addr_len)) ==
		ACCEPT_ERROR) {
#else
	if (xfer_sd < 0) {
#endif
		dprintf(D_ALWAYS, "I_accept() failed. (line %d)\n", __LINE__);
		exit(CHILDTERM_ACCEPT_ERROR);
    }
	close(data_conn_sd);

	if (socket_bufsize) {
			// Changing buffer size may fail
		setsockopt(xfer_sd, SOL_SOCKET, SO_SNDBUF, (char*) &socket_bufsize, 
			   sizeof(socket_bufsize));
	}

#ifdef WANT_NETMAN
	file_stream_info.startd_IP = chkpt_addr.sin_addr;
#endif
	bytes_sent = stream_file_xfer(file_fd, xfer_sd, file_size);

	close(xfer_sd);
	close(file_fd);

	// Write peer address to a temporary file which is read by the
	// parent.  The peer address is very important for logging purposes.
	sprintf(peer_info_filename, "/tmp/condor_ckpt_server.%d", getpid());
	peer_info_fd = safe_open_wrapper(peer_info_filename, O_WRONLY|O_CREAT|O_TRUNC, 0664);
	if (peer_info_fd >= 0) {
		write(peer_info_fd, (char *)&(chkpt_addr.sin_addr),
			  sizeof(struct in_addr));
		write(peer_info_fd, (char *)&bytes_sent, sizeof(bytes_sent));
		close(peer_info_fd);
	}

	if (bytes_sent != file_size) {
		dprintf(D_ALWAYS, "ERROR: Bad file size (sent %d, expected %d)\n",
				bytes_sent, file_size);
		exit(CHILDTERM_BAD_FILE_SIZE);
	}
}


void Server::ChildComplete()
{
	struct in_addr peer_addr;
	int			  peer_info_fd, xfer_size = 0;
	char		  peer_info_filename[100];
	int           child_pid;
	int           exit_status;
	int           exit_code;
	int           xfer_type;
	transferinfo* ptr;
	char          pathname[MAX_PATHNAME_LENGTH];
	char          log_msg[256];
	int           temp;
	bool	      success_flag = false;
	bool          at_least_one_child = false;

	memset((char *) &peer_addr, 0, sizeof(peer_addr));
	
	while ((child_pid=waitpid(-1, &exit_status, WNOHANG)) > 0) {
		at_least_one_child = true;
		if (WIFEXITED(exit_status)) {
			exit_code = WEXITSTATUS(exit_status);
		} else 	{
			// Must trap the SIGKILL signal explicitly since the reclaimation
			//   process may kill a process after it has completed its work.
			//   However, other signals will indicate some failure indicating
			//   abnormal termination
			if (WTERMSIG(exit_status) == SIGTERM)
				exit_code = CHILDTERM_KILLED;
			else
				exit_code = CHILDTERM_SIGNALLED;
		}
		xfer_type = transfers.GetXferType(child_pid);
		if (xfer_type == BAD_CHILD_PID) {
			dprintf(D_ALWAYS, "ERROR: Can not resolve child pid (#%d)\n",
					child_pid);
			exit(BAD_TRANSFER_LIST);
		}
		if ((ptr=transfers.Find(child_pid)) == NULL) {
			dprintf(D_ALWAYS, "ERROR: Can not resolve child pid (#%d)\n",
					child_pid);
			exit(BAD_TRANSFER_LIST);
		}
		switch (xfer_type) {
		    case RECV:
			    num_store_xfers--;
				if (exit_code == CHILDTERM_SUCCESS) {
					success_flag = true;
					Log(ptr->req_id, "File successfully received");
				} else if ((exit_code == CHILDTERM_KILLED) && 
						 (ptr->override == OVERRIDE)) {
					success_flag = true;
					Log(ptr->req_id, "File successfully received; kill signal");
					Log("overridden");
				} else {
					success_flag = false;
					if (exit_code == CHILDTERM_SIGNALLED) {
						sprintf(log_msg, 
								"File transfer terminated due to signal #%d",
								WTERMSIG(exit_status));
						Log(ptr->req_id, log_msg);
						if (WTERMSIG(exit_status) == SIGKILL)
							Log("User killed the peer process");
					}
					else if (exit_code == CHILDTERM_KILLED) {
						Log(ptr->req_id, 
							"File reception terminated by reclamation process");
					} else {
						sprintf(log_msg, 
								"File reception self-terminated; error #%d",
								exit_code);
						Log(ptr->req_id, log_msg);
						Log("occurred during file reception");
					}
					sprintf(pathname, "%s%s/%s/%s", LOCAL_DRIVE_PREFIX,
							inet_ntoa(ptr->shadow_addr), ptr->owner,
							ptr->filename);
					// Attempt to remove file
					if (remove(pathname) != 0)
						Log("Unable to remove file from physical storage");
					else
						Log("Partial file removed from physical storage");
					// Remove file information from in-memory data structure
					if ((temp=imds.RemoveFile(ptr->shadow_addr, ptr->owner, 
											  ptr->filename)) != REMOVED_FILE){
						sprintf(log_msg,
								"Unable to remove file from imds (%d)",
								temp);
						Log(log_msg);
					}
				}
				break;
			case XMIT:
				num_restore_xfers--;
				if (exit_code == CHILDTERM_SUCCESS) {
					success_flag = true;
					Log(ptr->req_id, "File successfully transmitted");
				} else if ((exit_code == CHILDTERM_KILLED) && 
						 (ptr->override == OVERRIDE)) {
					success_flag = true;
					Log(ptr->req_id,
						"File successfully transmitted; kill signal");
					Log("overridden");
				} else if (exit_code == CHILDTERM_SIGNALLED) {
					success_flag = false;
					sprintf(log_msg, 
							"File transfer terminated due to signal #%d",
							WTERMSIG(exit_status));
					Log(ptr->req_id, log_msg);
					if (WTERMSIG(exit_status) == SIGPIPE)
						Log("Socket on receiving end closed");
					else if (WTERMSIG(exit_status) == SIGKILL)
						Log("User killed the peer process");
				} else if (exit_code == CHILDTERM_KILLED) {
					success_flag = false;
					Log(ptr->req_id, 
						"File transmission terminated by reclamation");
					Log("process");
				} else {
					success_flag = false;
					sprintf(log_msg, 
							"File transmission self-terminated; error #%d", 
							exit_code);
					Log(ptr->req_id, log_msg);
					Log("occurred during file transmission");
				}
				break;
			case FILE_STATUS:
				if (exit_code == CHILDTERM_SUCCESS)
					Log(ptr->req_id, "Server status successfully transmitted");
				else if (exit_code == CHILDTERM_KILLED)
					Log(ptr->req_id, 
						"Server status terminated by reclamation process");
				else if (exit_code == CHILDTERM_SIGNALLED) {
					sprintf(log_msg, 
							"File transfer terminated due to signal #%d",
							WTERMSIG(exit_status));
					Log(ptr->req_id, log_msg);
					if (WTERMSIG(exit_status) == SIGPIPE)
						Log("Socket on receiving end closed");
					else if (WTERMSIG(exit_status) == SIGKILL)
						Log("User killed the peer process");
				} else {
					sprintf(log_msg, 
							"Server status self-terminated; error #%d",
							exit_code);
					Log(ptr->req_id, log_msg);
					Log("occurred during status transmission");
				}
				break;
			case REPLICATE:
				num_replicate_xfers--;
				if (exit_code == CHILDTERM_SUCCESS)
					Log(ptr->req_id, "File successfully replicated");
				else if ((exit_code == CHILDTERM_KILLED) && 
						 (ptr->override == OVERRIDE)) {
					Log(ptr->req_id,
						"File successfully replicated; kill signal");
					Log("overridden");
				} else if (exit_code == CHILDTERM_SIGNALLED) {
					sprintf(log_msg, 
							"File replication terminated due to signal #%d",
							WTERMSIG(exit_status));
					Log(ptr->req_id, log_msg);
					if (WTERMSIG(exit_status) == SIGPIPE)
						Log("Socket on receiving end closed");
					else if (WTERMSIG(exit_status) == SIGKILL)
						Log("User killed the peer process");
				} else if (exit_code == CHILDTERM_KILLED) {
					Log(ptr->req_id, 
						"File replication terminated by reclamation");
					Log("process");
				} else {
					sprintf(log_msg, 
							"File replication self-terminated; error #%d", 
							exit_code);
					Log(ptr->req_id, log_msg);
					Log("occurred during file transmission");
				}
				break;
			default:
				dprintf(D_ALWAYS, "ERROR: illegal transfer type used; terminating\n");
				exit(BAD_RETURN_CODE);	  
			}
		// Try to read the info file for this child to get the peer address.
		sprintf(peer_info_filename, "/tmp/condor_ckpt_server.%d", child_pid);
		peer_info_fd = safe_open_wrapper(peer_info_filename, O_RDONLY);
		if (peer_info_fd >= 0) {
			read(peer_info_fd, (char *)&peer_addr, sizeof(struct in_addr));
			read(peer_info_fd, (char *)&xfer_size, sizeof(xfer_size));
			close(peer_info_fd);
			unlink(peer_info_filename);
		}
		transfers.Delete(child_pid, success_flag, peer_addr, xfer_size);
    }
	
	// I should have processed at least one child. If not, then how did the
	// SIGCHLD signal hanlder get called but I have nothing to reap?
	if (child_pid < 0 && at_least_one_child == false) {
		dprintf(D_ALWAYS, "ERROR from waitpid(): %d (%s)\n", errno,
					strerror(errno));
	}
}


void Server::NoMore(char const *reason)
{
  more = 0;
  dprintf(D_ALWAYS, "%s: shutting down checkpoint server\n", 
  	reason==NULL?"(null)":reason);
}


void Server::ServerDump()
{
  imds.DumpInfo();
  transfers.Dump();
}


void Server::Log(int         request,
				 const char* event)
{
	dprintf(D_ALWAYS, "[reqid: %d] %s\n", request, event);
}


void Server::Log(const char* event)
{
	dprintf(D_ALWAYS, "    %s\n", event);
}


void InstallSigHandlers()
{
	struct sigaction sig_info;
	
	sig_info.sa_handler = (SIG_HANDLER) SigChildHandler;
	sigemptyset(&sig_info.sa_mask);
	sig_info.sa_flags = 0;
	if (sigaction(SIGCHLD, &sig_info, NULL) < 0) {
		dprintf(D_ALWAYS, "ERROR: cannot install SIGCHLD signal handler\n");
		exit(SIGNAL_HANDLER_ERROR);
    }
	sig_info.sa_handler = (SIG_HANDLER) SigUser1Handler;
	sigemptyset(&sig_info.sa_mask);
	sig_info.sa_flags = 0;
	if (sigaction(SIGUSR1, &sig_info, NULL) < 0) {
		dprintf(D_ALWAYS, "ERROR: cannot install SIGUSR1 signal handler\n");
		exit(SIGNAL_HANDLER_ERROR);
    }
	sig_info.sa_handler = (SIG_HANDLER) SigTermHandler;
	sigemptyset(&sig_info.sa_mask);
	sig_info.sa_flags = 0;
	if (sigaction(SIGTERM, &sig_info, NULL) < 0) {
		dprintf(D_ALWAYS, "ERROR: cannot install SIGTERM signal handler\n");
		exit(SIGNAL_HANDLER_ERROR);
    }
	sig_info.sa_handler = (SIG_HANDLER) SigQuitHandler;
	sigemptyset(&sig_info.sa_mask);
	sig_info.sa_flags = 0;
	if (sigaction(SIGQUIT, &sig_info, NULL) < 0) {
		dprintf(D_ALWAYS, "ERROR: cannot install SIGQUIT signal handler\n");
		exit(SIGNAL_HANDLER_ERROR);
    }
	sig_info.sa_handler = (SIG_HANDLER) SigHupHandler;
	sigemptyset(&sig_info.sa_mask);
	sig_info.sa_flags = 0;
	if (sigaction(SIGHUP, &sig_info, NULL) < 0) {
		dprintf(D_ALWAYS, "ERROR: cannot install SIGHUP signal handler\n");
		exit(SIGNAL_HANDLER_ERROR);
    }
	sig_info.sa_handler = (SIG_HANDLER) SIG_IGN;
	sigemptyset(&sig_info.sa_mask);
	sig_info.sa_flags = 0;
	if (sigaction(SIGPIPE, &sig_info, NULL) < 0) {
		dprintf(D_ALWAYS, "ERROR: cannot install SIGPIPE signal handler (SIG_IGN)\n");
		exit(SIGNAL_HANDLER_ERROR);
    }
	sig_info.sa_handler = (SIG_HANDLER) SigAlarmHandler;
	sigemptyset(&sig_info.sa_mask);
	sig_info.sa_flags = 0;
	if (sigaction(SIGALRM, &sig_info, NULL) < 0) {
		dprintf(D_ALWAYS, "ERROR: cannot install SIGALRM signal handler\n");
		exit(SIGNAL_HANDLER_ERROR);
    }

	BlockSignals();
}


void SigAlarmHandler(int)
{
  int saveErrno = errno;
  rt_alarm.Expired();
  errno = saveErrno;
}


void SigChildHandler(int)
{
  int saveErrno = errno;
  BlockSignals();
  server.ChildComplete();
  UnblockSignals();
  errno = saveErrno;
}


void SigUser1Handler(int)
{
  int saveErrno = errno;
  BlockSignals();
  cout << "SIGUSR1 trapped; this handler is incomplete" << endl << endl;
  server.ServerDump();
  UnblockSignals();
  errno = saveErrno;
}


void SigTermHandler(int)
{
  int saveErrno = errno;
  BlockSignals();
  cout << "SIGTERM trapped; Shutting down" << endl << endl;
  server.NoMore("SIGTERM trapped");
  UnblockSignals();
  errno = saveErrno;
}

void SigQuitHandler(int)
{
  int saveErrno = errno;
  BlockSignals();
  cout << "SIGQUIT trapped; Shutting down" << endl << endl;
  server.NoMore("SIGQUIT trapped");
  UnblockSignals();
  errno = saveErrno;
}


void SigHupHandler(int)
{
  int saveErrno = errno;
  BlockSignals();
  dprintf(D_ALWAYS, "SIGHUP trapped; Re-initializing\n");
  server.Init();
  UnblockSignals();
  errno = saveErrno;
}


void BlockSignals()
{
  sigset_t mask;

  if (sigprocmask(SIG_SETMASK, NULL, &mask) != 0)
    {
      dprintf(D_ALWAYS, "ERROR: cannot obtain current signal mask\n");
      exit(SIGNAL_MASK_ERROR);
    }
  if ((sigaddset(&mask, SIGCHLD) + sigaddset(&mask, SIGUSR1) +
       sigaddset(&mask, SIGTERM) + sigaddset(&mask, SIGPIPE)) != 0)
    {
	  dprintf(D_ALWAYS, "ERROR: cannot add signals to signal mask\n");
      exit(SIGNAL_MASK_ERROR);
    }
  if (sigprocmask(SIG_SETMASK, &mask, NULL) != 0)
    {
      dprintf(D_ALWAYS, "ERROR: cannot set new signal mask\n");
      exit(SIGNAL_MASK_ERROR);
    }
}


void UnblockSignals()
{
  sigset_t mask;

  if (sigprocmask(SIG_SETMASK, NULL, &mask) != 0)
    {
      dprintf(D_ALWAYS, "ERROR: cannot obtain current signal mask\n");
      exit(SIGNAL_MASK_ERROR);
    }
  if ((sigdelset(&mask, SIGCHLD) + sigdelset(&mask, SIGUSR1) +
       sigdelset(&mask, SIGTERM) + sigdelset(&mask, SIGPIPE)) != 0)
    {
      dprintf(D_ALWAYS, "ERROR: cannot remove signals from signal mask\n");
      exit(SIGNAL_MASK_ERROR);
    }
  if (sigprocmask(SIG_SETMASK, &mask, NULL) != 0)
    {
      dprintf(D_ALWAYS, "ERROR: cannot set new signal mask\n");
      exit(SIGNAL_MASK_ERROR);
    }
}


int main( int argc, char **argv )
{
	myName = argv[0];
	server.Init();
	server.Execute();
	return 0;                            // Should never execute
}