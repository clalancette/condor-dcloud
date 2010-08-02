if(NOT WIN_EXEC_NODE_ONLY)

	file( GLOB bgpScripts bgp_*)
	file( GLOB OtherSrc htc_*)

	condor_glob(startdFHdrs startdFSrcs "${bgpScripts};${OtherSrc}")

	condor_exe( condor_startd_factory "${startdFHdrs};${startdFSrcs}" ${C_SBIN} "${CONDOR_LIBS};${PCRE_FOUND};${OPENSSL_FOUND};${KRB5_FOUND};${CLASSADS_FOUND};${COREDUMPER_FOUND}")

	message(STATUS "TODO: TSTCLAIR add scripts to install targets")

	#Install source file	
	file( GLOB htcSrc htc_tool.cpp htc_list_partitions.cpp)
	install(FILES		${htcSrc}
		DESTINATION	"${C_SRC}/startd_factory/BlueGeneP/")


endif(NOT WIN_EXEC_NODE_ONLY)

