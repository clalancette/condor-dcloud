if (NOT WIN_EXEC_NODE_ONLY)
	condor_exe( condor_shared_port "shared_port_main.cpp" ${C_LIBEXEC} "${CONDOR_LIBS};${PCRE_FOUND};${OPENSSL_FOUND};${KRB5_FOUND};${CLASSADS_FOUND};${COREDUMPER_FOUND}")	
endif(NOT WIN_EXEC_NODE_ONLY)	