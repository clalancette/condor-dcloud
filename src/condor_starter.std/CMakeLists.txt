if(STD_UNIVERSE )

	# include the other libs.
	file( GLOB RmvElements test* driver.cpp)
        
	src_target_ref(ckpt "syscall_mode;signals_control" CKPT_REFS)
	src_target_ref(condorsyscall "remote_syscall;syscall_connect;syscall_numbers;xfer_file;senders;file_stream" CNDRSYSCL_REFS)

	condor_glob(Hdrs Srcs "${RmvElements}")

	condor_exe(condor_starter.std
	"${Hdrs};${Srcs};${CKPT_REFS};${CNDRSYSCL_REFS}" ${C_SBIN}
	"${CONDOR_TOOL_LIBS};ckpt_server_api;stdunivio;${KRB5_FOUND};${OPENSSL_FOUND};${PCRE_FOUND};${ZLIB_FOUND}")

	add_dependencies( condor_starter.std ckpt condorsyscall )


endif(STD_UNIVERSE)
