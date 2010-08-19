if (NOT WIN_EXEC_NODE_ONLY)

	condor_selective_glob("StateMachine.*;HAD.*;Utils.*" HADSrcs)

	condor_exe(condor_had "${HADSrcs}" ${C_SBIN} "${PCRE_FOUND};${CONDOR_LIBS};${OPENSSL_FOUND};${COREDUMPER_FOUND};${KRB5_FOUND}")

	condor_selective_glob("AbstractReplicatorStateMachine.*;ReplicatorStateMachine.*;HAD_Version.*;Utils.*;Replication.*;FilesOperations.*" RepSrcs)

	condor_exe(condor_replication "${RepSrcs}" ${C_SBIN} "${PCRE_FOUND};${CONDOR_LIBS};${OPENSSL_FOUND};${COREDUMPER_FOUND};${KRB5_FOUND}")

	condor_selective_glob("BaseReplicaTransferer.*;UploadReplicaTransferer.*;DownloadReplicaTransferer.*;Utils.*;Transferer.*;FilesOperations.*" TransferSrcs)

	condor_exe(condor_transferer "${TransferSrcs}" ${C_SBIN} "${PCRE_FOUND};${CONDOR_LIBS};${OPENSSL_FOUND};${COREDUMPER_FOUND};${KRB5_FOUND}")

endif(NOT WIN_EXEC_NODE_ONLY)