 ###############################################################
 # 
 # Copyright 2011 Red Hat, Inc. 
 # 
 # Licensed under the Apache License, Version 2.0 (the "License"); you 
 # may not use this file except in compliance with the License.  You may 
 # obtain a copy of the License at 
 # 
 #    http://www.apache.org/licenses/LICENSE-2.0 
 # 
 # Unless required by applicable law or agreed to in writing, software 
 # distributed under the License is distributed on an "AS IS" BASIS, 
 # WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 # See the License for the specific language governing permissions and 
 # limitations under the License. 
 # 
 ############################################################### 


file( GLOB ProcdRmvElements *_test* proc_family_client* proc_family_io*
local_client.UNIX* procd_ctl* gidd_alloc*)

if (WINDOWS)
	set (PROCD_WIN_LINK_LIBS psapi)
	condor_selective_glob ( "proc_family_client*;proc_family_io*;local_client.*" ProcClientElements)
else(WINDOWS)
    condor_selective_glob( "proc_family_client*;proc_family_io*;local_client.*;named_pipe_reader*;named_pipe_writer*;named_pipe_util*;named_pipe_watchdog*" ProcClientElements )
endif(WINDOWS)

condor_daemon(procd "${ProcdRmvElements}" "procapi;utils;${PROCD_WIN_LINK_LIBS}" "${C_SBIN}" OFF)

if (LINUX AND WANT_FULL_DEPLOYMENT)
	condor_exe( procd_ctl "procd_ctl.cpp" ${C_SBIN} "${CONDOR_TOOL_LIBS};${PCRE_FOUND};${OPENSSL_FOUND};${KRB5_FOUND};${COREDUMPER_FOUND}" OFF)

	condor_exe( gidd_alloc "gidd_alloc.cpp" ${C_SBIN} "${CONDOR_TOOL_LIBS};${PCRE_FOUND};${OPENSSL_FOUND};${KRB5_FOUND};${COREDUMPER_FOUND}" OFF)
endif(LINUX AND WANT_FULL_DEPLOYMENT)

condor_static_lib(procd_client "${ProcClientElements}")
