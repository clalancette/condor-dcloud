
if (NOT WIN_EXEC_NODE_ONLY)

	## create targets
	file( GLOB collectorRmvElements Example* )
	condor_daemon ( collector
					"${collectorRmvElements}"
					"${CONDOR_LIBS};${GSOAP_FOUND};${PCRE_FOUND};${OPENSSL_FOUND};${COREDUMPER_FOUND};${KRB5_FOUND};${ZLIB_FOUND}"
					${C_SBIN}
					ON )

	install ( FILES condorCollector.wsdl DESTINATION ${C_LIB}/webservice )

endif(NOT WIN_EXEC_NODE_ONLY)




