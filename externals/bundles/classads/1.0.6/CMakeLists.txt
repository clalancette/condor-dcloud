
#enable once we fix *this
#option( WITH_CLASSADS "Compiling with support for CLASSADS" ON )

if (WITH_CLASSADS)

	if ( NOT PROPER )

		ExternalProject_Add(classads
							PREFIX classads-1.0.6
							#-- Download Step ----------
							DOWNLOAD_COMMAND wget -N http://parrot.cs.wisc.edu/externals/classads-1.0.6.tar.gz ${CMD_TERM}
							DOWNLOAD_DIR ${EXTERNAL_DL}
							URL http://parrot.cs.wisc.edu/externals/classads-1.0.6.tar.gz
							#--Configure step ----------
							CONFIGURE_COMMAND ./configure --without-pcre --disable-shared --enable-static --enable-namespace --prefix=${EXTERNAL_STAGE} CC=g++
							#--Build Step ----------
							BUILD_COMMAND make && make check
							BUILD_IN_SOURCE 1
							#--install Step ----------
							INSTALL_DIR ${EXTERNAL_STAGE}
							INSTALL_COMMAND make install)

		# Set the target dependencies which the rest of condor depends on.
		set(CLASSADS_FOUND "${EXTERNAL_STAGE}/lib/libclassad.a;${EXTERNAL_STAGE}/lib/libclassad_ns.a")

	else( NOT PROPER )

		#find the following libraries.
		find_multiple( "classad;classad_ns" CLASSADS_FOUND)

	endif( NOT PROPER )

	## Set all the config information
	set( CLASSADS_FOUND ${CLASSADS_FOUND} PARENT_SCOPE )
	set( HAVE_EXT_CLASSADS ON PARENT_SCOPE )

	message (STATUS "external configured (CLASSADS_FOUND=${CLASSADS_FOUND})")

else(WITH_CLASSADS)

	#message (STATUS "external skiped (classads)")

	#this is a temporary hack b/c external != internal and there are 2 sources.. :-/
	set( CLASSADS_FOUND classads )
	set( CLASSADS_FOUND ${CLASSADS_FOUND} PARENT_SCOPE )
	set( HAVE_EXT_CLASSADS ON PARENT_SCOPE )
	message (STATUS "external configured (CLASSADS_FOUND=${CLASSADS_FOUND}) *internal ref non-proper :-/* ")

endif(WITH_CLASSADS)
