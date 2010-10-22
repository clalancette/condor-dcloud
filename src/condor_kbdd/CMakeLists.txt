
if ( WINDOWS OR ${HAVE_X11} )

	condor_glob(kbdHdrs kbdSrcs "")

	condor_exe( condor_kbdd "${kbdHdrs};${kbdSrcs}" ${C_SBIN} "${PCRE_FOUND};${CONDOR_LIBS};${OPENSSL_FOUND};${COREDUMPER_FOUND};${KRB5_FOUND};${HAVE_X11}")

endif()
