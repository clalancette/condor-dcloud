
#file( GLOB HOOK hdfs_plugin.* hdfs_plugin_main.*)
#file( GLOB HDFSLIB hdfs.* hdfsJniHelper.* )

#add_library( hdfs SHARED ${HDFSLIB} )

#condor_exe( condor_hdfs_hook "${HOOK}" ${C_SBIN} "hdfs;${CONDOR_TOOL_LIBS};${PCRE_FOUND};${OPENSSL_FOUND};${KRB5_FOUND};${CLASSADS_FOUND};${COREDUMPER_FOUND}")

message(STATUS "TODO: TSTCLAIR Deal with Java deps and FT HOOK PLUGINS")
