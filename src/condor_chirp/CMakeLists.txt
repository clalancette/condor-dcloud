

condor_static_lib( chirp_client "chirp_client.h;chirp_client.cpp" )

condor_exe( condor_chirp "condor_chirp.cpp" ${C_LIBEXEC}
"chirp_client;${CONDOR_TOOL_LIBS};${PCRE_FOUND};${OPENSSL_FOUND};${KRB5_FOUND};${GCB_FOUND};${GLOBUS_FOUND}" )

install ( FILES Chirp.jar DESTINATION ${C_LIB} )

if (NOT WINDOWS)
	install (TARGETS chirp_client DESTINATION ${C_LIB})
	install (FILES chirp_client.cpp chirp_client.h chirp_protocol.h DESTINATION ${C_SRC}/chirp)
endif (NOT WINDOWS)