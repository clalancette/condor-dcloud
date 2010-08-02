if ( HAVE_EXT_CREAM )

	message(STATUS "TODO: cream non-proper")
	condor_exe( cream_gahp
				"cream_gahp_server.cpp"
				${C_SBIN}
				"" )
else()

	message(STATUS "NOTE: *cream* targets will not be built HAVE_EXT_CREAM=${HAVE_EXT_CREAM}")

endif (HAVE_EXT_CREAM)