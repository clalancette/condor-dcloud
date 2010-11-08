if ( HAVE_EXT_CREAM AND NOT PROPER )

	# cream is such an odd beast and it collides with other versions of
	# externals
	include_directories ( ${CREAM_STAGE}/include )
	link_directories ( ${CREAM_STAGE}/lib )
	add_definitions(-D_REENTRANT)
	
	condor_exe( cream_gahp
		"cream_gahp_server.cpp"
		${C_SBIN}
		"${CREAM_FOUND};${OPENSSL_FOUND};${EXPAT_FOUND};${GLOBUS_THR_FOUND};${VOMS_FOUND};${CLASSADS_FOUND}" )
else()

	message(STATUS "NOTE: *cream* targets will not be built HAVE_EXT_CREAM=${HAVE_EXT_CREAM}")

endif()
