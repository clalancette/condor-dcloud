
file( GLOB ProcdRmvElements *_test* proc_family_client* proc_family_io* local_client.UNIX* procd_ctl*)


if (WINDOWS)
	set (PROCD_WIN_LINK_LIBS psapi) 
		
	file( GLOB ProcClientElements proc_family_client*
                                  proc_family_io*
                                  local_client.WIN*
                                  local_client.h)
	
else()
	

    # TBD: clean this portion up.
	file( GLOB ProcClientElements proc_family_client*
                                  proc_family_io*
                                  local_client.UNIX*
								  local_client.h
                                  named_pipe_reader*
                                  named_pipe_writer*
                                  named_pipe_util*
                                  named_pipe_watchdog*)

endif()

condor_daemon(procd "${ProcdRmvElements}" "procapi;utils;${PROCD_WIN_LINK_LIBS}" "${C_SBIN}" OFF)

if (WINDOWS)
	condor_exe( procd_ctl "procd_ctl.cpp" ${C_SBIN} "${CONDOR_LIBS};${PCRE_FOUND};${OPENSSL_FOUND};${KRB5_FOUND};${COREDUMPER_FOUND}")
endif(WINDOWS)

condor_static_lib(procd_client "${ProcClientElements}")