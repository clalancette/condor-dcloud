if( LINUX AND NOT CLIPPED AND GLIBC_VERSION AND NOT PROPER )

	# include the other libs.
	condor_daemon( starter.std "" "${CONDOR_LIBS}" "${C_SBIN}" OFF )

endif(LINUX AND NOT CLIPPED AND GLIBC_VERSION AND NOT PROPER )