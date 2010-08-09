condor_glob( UTHdrs UTSrcs test_daemon_core.cpp )

condor_exe_test(condor_unit_tests "${UTSrcs}" "${CONDOR_TOOL_LIBS};${COREDUMPER_FOUND};${PCRE_FOUND}")