option(WITH_ZLIB "Compiling with support for ZLIB" ON)

if (WITH_ZLIB)

	if ( NOT PROPER )

		set (ZLIB_VER zlib-1.2.3)

		ExternalProject_Add(zlib
				    PREFIX ${EXTERNAL_BUILD_PREFIX}/${ZLIB_VER}
				    #-- Download Step ----------
					DOWNLOAD_COMMAND wget -N http://parrot.cs.wisc.edu/externals/${ZLIB_VER}.tar.gz ${CMD_TERM}
				    DOWNLOAD_DIR ${EXTERNAL_DL}
				    URL http://parrot.cs.wisc.edu/externals/${ZLIB_VER}.tar.gz
				    #--Configure step ----------
				    CONFIGURE_COMMAND cd ${ZLIB_VER} && ./configure --prefix=${EXTERNAL_STAGE}
				    #--Build Step ----------
				    BUILD_COMMAND cd ${ZLIB_VER} && make
				    BUILD_IN_SOURCE 1
				    #--install Step ----------
				    INSTALL_DIR ${EXTERNAL_STAGE}
				    INSTALL_COMMAND cd ${ZLIB_VER} && make install )

		# Set the target dependencies which the rest of condor depends on.
		set(ZLIB_FOUND "${EXTERNAL_STAGE}/lib/libz.a")
		append_var(CONDOR_EXTERNALS zlib)

	else( NOT PROPER )

		find_multiple( "z" ZLIB_FOUND )

	endif( NOT PROPER )

	message (STATUS "external configured (ZLIB_FOUND=${ZLIB_FOUND})")
	set( ZLIB_FOUND ${ZLIB_FOUND} PARENT_SCOPE )
	set( HAVE_EXT_ZLIB ON PARENT_SCOPE )

else(WITH_ZLIB)

	message (STATUS "external skiped (zlib)")

endif(WITH_ZLIB)