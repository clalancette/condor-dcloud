
if (NOT WINDOWS)
	if (WANT_LEASE_MANAGER)

		file( GLOB rmvLMFiles *test*)

		condor_glob(LMHdrs LMSrcs "${rmvLMFiles}*")

		condor_exe( condor_lease_manager "${LMHdrs};${LMSrcs}" ${C_SBIN}
		"conversion;${CONDOR_LIBS};${PCRE_FOUND};${OPENSSL_FOUND};${KRB5_FOUND};${CLASSADS_FOUND};${COREDUMPER_FOUND}")

		add_dependencies( condor_lease_manager conversion)

	else(WANT_LEASE_MANAGER)
		message(STATUS "NOTE: *lease_manager* targets will not be built WANT_LEASE_MANAGER=${WANT_LEASE_MANAGER}")
	endif(WANT_LEASE_MANAGER)
endif()
