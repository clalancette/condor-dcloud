
if (DARWIN)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework IOKit -framework Carbon")
endif()

file( GLOB startdRmvElements Example* *.t.* glexec_wrapper*)

condor_daemon( startd "${startdRmvElements}"
"${CONDOR_LIBS};${PCRE_FOUND};${OPENSSL_FOUND};${COREDUMPER_FOUND};${KRB5_FOUND};${GLOBUS_FOUND};${GCB_FOUND}" "${C_SBIN}" OFF)





