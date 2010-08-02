if (WINDOWS)

	condor_exe( condor_credd
				"credd.windows.cpp;credd.windows.h"
				${C_SBIN}
				"${CONDOR_LIBS};${PCRE_FOUND};${OPENSSL_FOUND};${KRB5_FOUND}" )
				
else(WINDOWS)

	if ( HAVE_EXT_CLASSADS )

		file( GLOB CREDRMVSRCS *_cred* )
		condor_glob( CREDDHDRS CREDDSRCS "${CREDRMVSRCS}" )
		condor_exe( condor_credd
			"${CREDDHDRS};${CREDDSRCS}"
			${C_SBIN}
			"${CONDOR_LIBS};${PCRE_FOUND};${OPENSSL_FOUND};${COREDUMPER_FOUND};${KRB5_FOUND}" )

		message(STATUS "TODO: TSTCLAIR credd->stork targets?")
		
	else ()

		message(STATUS "NOTE: *credd* targets will not be built HAVE_EXT_GLOBUS=${HAVE_EXT_GLOBUS} && HAVE_EXT_CLASSADS=${HAVE_EXT_CLASSADS}")

	endif()
	
endif (WINDOWS)
