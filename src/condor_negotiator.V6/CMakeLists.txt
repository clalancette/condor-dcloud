if (NOT WIN_EXEC_NODE_ONLY)

	file( GLOB negotiatorRmvElements Example* )
	condor_daemon( negotiator "${negotiatorRmvElements}"
	"${CONDOR_LIBS};${PCRE_FOUND};${OPENSSL_FOUND};${COREDUMPER_FOUND};${KRB5_FOUND};${GLOBUS_FOUND};${GCB_FOUND}" "${C_SBIN}" OFF )

endif(NOT WIN_EXEC_NODE_ONLY)



