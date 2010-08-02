condor_exe( condor_rm "rm.cpp" ${C_BIN} "${CONDOR_TOOL_LIBS};${PCRE_FOUND};${OPENSSL_FOUND};${KRB5_FOUND};${CLASSADS_FOUND};${COREDUMPER_FOUND}")
clone_install( condor_rm condor_hold ${C_BIN} )
clone_install( condor_rm condor_release ${C_BIN} )
clone_install( condor_rm condor_vacate_job ${C_BIN} )
