if (NOT WIN_EXEC_NODE_ONLY)

	file( GLOB CGahpSrcs Pipe*.h Pipe*.cpp io*.h io*.cpp cgahp*.h
	cgahp*.cpp )

	condor_exe( condor_c-gahp "${CGahpSrcs}" ${C_SBIN}
	"qmgmt;${CONDOR_LIBS};${PCRE_FOUND};${OPENSSL_FOUND};${KRB5_FOUND};${COREDUMPER_FOUND}" )

	file( GLOB CGahpWorkerSrcs Pipe*.h Pipe*.cpp schedd*.h schedd*.cpp
	Sched*.h Sched*.cpp cgahp*.h cgahp*.cpp)

	condor_exe( condor_c-gahp_worker_thread "${CGahpWorkerSrcs}" ${C_SBIN}
	"qmgmt;${CONDOR_LIBS};${PCRE_FOUND};${OPENSSL_FOUND};${KRB5_FOUND};${COREDUMPER_FOUND}" )

endif(NOT WIN_EXEC_NODE_ONLY)
