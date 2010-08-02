condor_glob( edHdrs edSrcs "" )

add_executable( condor_eventd ${edHdrs} ${edSrcs} )

condor_set_link_libs( condor_eventd "${PCRE_FOUND};${CONDOR_LIBS};${OPENSSL_FOUND};${COREDUMPER_FOUND};${KRB5_FOUND}")

message (STATUS "TODO: TSTCLAIR why is eventd not installed?")