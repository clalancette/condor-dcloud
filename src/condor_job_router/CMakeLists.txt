

set (JR_LINK_LIBS "qmgmt;${CONDOR_LIBS};${PCRE_FOUND};${OPENSSL_FOUND};${KRB5_FOUND};${CLASSADS_FOUND};${COREDUMPER_FOUND}")

condor_glob(JRHdrs JRSrcs "VanillaToGrid_main.cpp")

condor_exe( condor_job_router "${JRHdrs};${JRSrcs}" ${C_LIBEXEC} "${JR_LINK_LIBS}")
condor_exe( condor_router_q "${JRHdrs};${JRSrcs}" ${C_BIN} "${JR_LINK_LIBS}")
condor_exe( condor_router_rm "${JRHdrs};${JRSrcs}" ${C_BIN} "${JR_LINK_LIBS}")
condor_exe( condor_router_history "${JRHdrs};${JRSrcs}" ${C_BIN} "${JR_LINK_LIBS}")
