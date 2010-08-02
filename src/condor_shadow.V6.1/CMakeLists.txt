
## Create targets
file( GLOB shadowRmvElements Example* *.t.* )
condor_daemon( shadow "${shadowRmvElements}" "qmgmt;${CONDOR_LIBS};${PCRE_FOUND};${OPENSSL_FOUND};${COREDUMPER_FOUND};${KRB5_FOUND}" "${C_SBIN}" OFF )


