
condor_exe( condor_userlog "userlog.cpp" ${C_BIN} "${CONDOR_TOOL_LIBS};${PCRE_FOUND};${OPENSSL_FOUND};${KRB5_FOUND};${CLASSADS_FOUND};${COREDUMPER_FOUND}")

condor_exe( condor_check_userlogs "condor_check_userlogs.cpp" ${C_BIN} "${CONDOR_TOOL_LIBS};${PCRE_FOUND};${OPENSSL_FOUND};${KRB5_FOUND};${CLASSADS_FOUND};${COREDUMPER_FOUND}")

if (NOT WINDOWS)
	condor_exe( condor_userlog_job_counter "condor_userlog_job_counter.cpp" ${C_BIN} "${CONDOR_TOOL_LIBS};${PCRE_FOUND};${OPENSSL_FOUND};${KRB5_FOUND};${CLASSADS_FOUND};${COREDUMPER_FOUND}")
endif()