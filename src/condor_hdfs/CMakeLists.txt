if (NOT WIN_EXEC_NODE_ONLY AND HAVE_EXT_HADOOP)

	condor_glob(HDFSHdrs HDFSSrcs "")

	condor_exe( condor_hdfs "${HDFSHdrs};${HDFSSrcs}" ${C_SBIN} "${CONDOR_LIBS};${PCRE_FOUND};${OPENSSL_FOUND};${KRB5_FOUND};${CLASSADS_FOUND};${COREDUMPER_FOUND}")

endif()
