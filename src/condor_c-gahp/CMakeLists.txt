if (NOT WIN_EXEC_NODE_ONLY)

	condor_selective_glob( "Pipe*;io*;cgahp*" CGahpSrcs )

	condor_exe( condor_c-gahp "${CGahpSrcs}" ${C_SBIN} "qmgmt;${CONDOR_LIBS};${PCRE_FOUND};${OPENSSL_FOUND};${KRB5_FOUND};${COREDUMPER_FOUND}" )

	condor_selective_glob("Pipe*;schedd*;Sched*;cgahp*" CGahpWorkerSrcs)

	condor_exe( condor_c-gahp_worker_thread "${CGahpWorkerSrcs}" ${C_SBIN} "qmgmt;${CONDOR_LIBS};${PCRE_FOUND};${OPENSSL_FOUND};${KRB5_FOUND};${COREDUMPER_FOUND}" )

endif(NOT WIN_EXEC_NODE_ONLY)
