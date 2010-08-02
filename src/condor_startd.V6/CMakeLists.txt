
file( GLOB startdRmvElements Example* *.t.* glexec_wrapper*)

condor_daemon( startd "${startdRmvElements}"
"${CONDOR_LIBS};${PCRE_FOUND};${OPENSSL_FOUND};${COREDUMPER_FOUND};${KRB5_FOUND};${GLOBUS_FOUND};${GCB_FOUND}" "${C_SBIN}" OFF)





