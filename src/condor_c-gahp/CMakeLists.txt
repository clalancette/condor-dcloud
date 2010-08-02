if (NOT WIN_EXEC_NODE_ONLY)

	file( GLOB CGahpSrcs PipeBuffer.* io_loop.* cgahp_common.* )

	condor_exe( condor_c-gahp "${CGahpSrcs}" ${C_SBIN} "qmgmt;${CONDOR_LIBS};${PCRE_FOUND};${OPENSSL_FOUND};${KRB5_FOUND};${LIBVIRT_FOUND};${COREDUMPER_FOUND}" )

	file( GLOB CGahpWorkerSrcs PipeBuffer.* schedd_client.* schedd_client_main.* SchedDCommands.* cgahp_common.*)

	condor_exe( condor_c-gahp_worker_thread "${CGahpWorkerSrcs}" ${C_SBIN} "qmgmt;${CONDOR_LIBS};${PCRE_FOUND};${OPENSSL_FOUND};${KRB5_FOUND};${LIBVIRT_FOUND};${COREDUMPER_FOUND}" )

endif(NOT WIN_EXEC_NODE_ONLY)