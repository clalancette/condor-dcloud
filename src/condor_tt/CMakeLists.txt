##############
if (HAVE_EXT_POSTGRESQL)
	condor_selective_glob("condor_ttdb*;jobqueuecollection*" ttLibSrcs)

	condor_glob( ttHdrs ttSrcs "${ttLibSrcs}" )

	condor_static_lib( tt "${ttLibSrcs}" )

	condor_exe(condor_quill "${ttHdrs};${ttSrcs}" OFF "tt;${PCRE_FOUND};${CONDOR_LIBS};${OPENSSL_FOUND};${COREDUMPER_FOUND};${KRB5_FOUND};${POSTGRESQL_FOUND}")
else(HAVE_EXT_POSTGRESQL)
	message(STATUS "NOTE: tt targets will not be built HAVE_EXT_POSTGRESQL=${HAVE_EXT_POSTGRESQL}")
endif(HAVE_EXT_POSTGRESQL)