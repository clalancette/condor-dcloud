condor_glob(statusHdrs statusSrcs "")

condor_exe( condor_status "${statusHdrs};${statusSrcs}" ${C_BIN}
"${CONDOR_TOOL_LIBS};${PCRE_FOUND};${OPENSSL_FOUND};${KRB5_FOUND};${CLASSADS_FOUND};${COREDUMPER_FOUND};${GLOBUS_FOUND};${GCB_FOUND}" )
