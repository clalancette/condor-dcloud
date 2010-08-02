
if(NOT WIN_EXEC_NODE_ONLY)
	set (GM_LINK_LIBS "${CONDOR_TOOL_LIBS};${PCRE_FOUND};${OPENSSL_FOUND};${KRB5_FOUND};${COREDUMPER_FOUND}")

	file( GLOB RmvSrcs *_gahp_wrapper* )
	condor_glob( GMHDRS GMSRCS "${RmvSrcs}" )

	condor_exe( condor_gridmanager "${GMHDRS};${GMSRCS}" ${C_SBIN} "qmgmt;daemon_core;${GM_LINK_LIBS}" )

	if (NOT WINDOWS)
		condor_exe( gt4_gahp "gt4_gahp_wrapper.cpp" ${C_SBIN} "${GM_LINK_LIBS}" )
		condor_exe( gt42_gahp "gt42_gahp_wrapper.cpp" ${C_SBIN} "${GM_LINK_LIBS}" )
		condor_exe( unicore_gahp "unicore_gahp_wrapper.cpp" ${C_SBIN} "${GM_LINK_LIBS}" )
	endif()

	message(STATUS "TODO: add scripts to release")

endif(NOT WIN_EXEC_NODE_ONLY)