if (NOT WINDOWS)
	condor_glob(transHdrs transSrcs "td_instantiate.cpp")

	condor_exe( condor_transferd "${transHdrs};${transSrcs}" ${C_SBIN} "${PCRE_FOUND};${CONDOR_LIBS};${OPENSSL_FOUND};${COREDUMPER_FOUND};${KRB5_FOUND}")
endif(NOT WINDOWS)