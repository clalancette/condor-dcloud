option(WITH_EXPAT "Compiling with support for EXPAT" ON)

if ( WITH_EXPAT )

	if( NOT PROPER )

		set ( EXPAT_VER expat-2.0.1 )

		ExternalProject_Add(expat
				    PREFIX ${EXTERNAL_BUILD_PREFIX}/${EXPAT_VER}
				    #-- Download Step ----------
					DOWNLOAD_COMMAND wget -N http://parrot.cs.wisc.edu/externals/${EXPAT_VER}.tar.gz ${CMD_TERM}
				    DOWNLOAD_DIR ${EXTERNAL_DL}
				    URL http://parrot.cs.wisc.edu/externals/${EXPAT_VER}.tar.gz
				    #--Configure step ----------
				    CONFIGURE_COMMAND cd ${EXPAT_VER} && ./configure --prefix=${EXTERNAL_STAGE}
				    #--Build Step ----------
				    BUILD_COMMAND cd ${EXPAT_VER} && make
				    BUILD_IN_SOURCE 1
				    #--install Step ----------
				    INSTALL_DIR ${EXTERNAL_STAGE}
				    INSTALL_COMMAND cd ${EXPAT_VER} && make install )

		# Set the target dependencies which the rest of condor depends on.
		set(EXPAT_FOUND "${EXTERNAL_STAGE}/lib/libexpat.a")
		append_var (CONDOR_EXTERNALS expat)

	else( NOT PROPER )

		find_multiple( "expat" EXPAT_FOUND )

	endif( NOT PROPER )

	if (EXPAT_FOUND)
		message (STATUS "external configured (EXPAT_FOUND=${EXPAT_FOUND})")
		set( EXPAT_FOUND ${EXPAT_FOUND} PARENT_SCOPE )
		set( HAVE_EXT_EXPAT ON PARENT_SCOPE )
	endif (EXPAT_FOUND)

else(WITH_EXPAT)

	message (STATUS "external skiped (expat)")

endif(WITH_EXPAT)