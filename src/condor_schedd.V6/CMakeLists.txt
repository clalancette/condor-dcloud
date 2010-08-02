

file( GLOB qmgmtElements qmgmt_send_stubs* qmgr_lib_support* qmgr_job_updater* )
file( GLOB scheddRmvElements Example* *.t.* ${qmgmtElements} )

if (NOT WIN_EXEC_NODE_ONLY)
	condor_daemon( schedd "${scheddRmvElements}"
	"ckpt_server_api;${CONDOR_LIBS};${GSOAP_FOUND};${PCRE_FOUND};${OPENSSL_FOUND};${COREDUMPER_FOUND};${KRB5_FOUND};${ZLIB_FOUND}" "${C_SBIN}" ON)

	install (FILES condorSchedd.wsdl DESTINATION ${C_LIB}/webservice)
endif(NOT WIN_EXEC_NODE_ONLY)

condor_static_lib( qmgmt "${qmgmtElements};qmgmt_common.cpp" )







