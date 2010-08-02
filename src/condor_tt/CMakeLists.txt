##############
if (HAVE_EXT_POSTGRESQL)

	file( GLOB ttLibSrcs condor_ttdb* jobqueuecollection* )

	condor_glob( ttHdrs ttSrcs "${ttLibSrcs}" )

	condor_static_lib( tt "${ttLibSrcs}" )

	condor_exe(condor_quill "${ttHdrs};${ttSrcs}" ${C_SBIN} "tt;${PCRE_FOUND};${CONDOR_LIBS};${OPENSSL_FOUND};${COREDUMPER_FOUND};${KRB5_FOUND};${POSTGRESQL_FOUND}")
else()
	message(STATUS "NOTE: tt targets will not be built HAVE_EXT_POSTGRESQL=!DEFINED")
endif()