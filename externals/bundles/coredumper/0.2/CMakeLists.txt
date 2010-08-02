option(WITH_COREDUMPER "Compiling with support for COREDUMPER" ON)

if (WITH_COREDUMPER)

	if ( NOT PROPER )

		set (CRDPR_VER coredumper-0.2)

		ExternalProject_Add(coredumper
				    PREFIX ${EXTERNAL_BUILD_PREFIX}/${CRDPR_VER}
				    #-- Download Step ----------
					DOWNLOAD_COMMAND wget -N http://parrot.cs.wisc.edu/externals/${CRDPR_VER}.tar.gz ${CMD_TERM}
				    DOWNLOAD_DIR ${EXTERNAL_DL}
				    URL http://parrot.cs.wisc.edu/externals/${CRDPR_VER}.tar.gz
				    #--Configure step ----------
				    CONFIGURE_COMMAND cd ${CRDPR_VER} && ./configure --prefix=${EXTERNAL_STAGE}
				    #--Build Step ----------
				    BUILD_COMMAND cd ${CRDPR_VER} && make
				    BUILD_IN_SOURCE 1
				    #--install Step ----------
				    INSTALL_DIR ${EXTERNAL_STAGE}
				    INSTALL_COMMAND cd ${CRDPR_VER} && make install )

		# Set the target dependencies which the rest of condor depends on.
		set(COREDUMPER_FOUND "${EXTERNAL_STAGE}/lib/libcoredumper.a")
		append_var (CONDOR_EXTERNALS coredumper)

	else( NOT PROPER )

		find_multiple( "coredumper" COREDUMPER_FOUND )

	endif( NOT PROPER )

	if (COREDUMPER_FOUND)
		message (STATUS "external configured (COREDUMPER_FOUND=${COREDUMPER_FOUND})")
		set( COREDUMPER_FOUND ${COREDUMPER_FOUND} PARENT_SCOPE )
		set( HAVE_EXT_COREDUMPER ON PARENT_SCOPE )
	endif(COREDUMPER_FOUND)

else(WITH_COREDUMPER)

	message (STATUS "external skiped (coredumper)")

endif(WITH_COREDUMPER)