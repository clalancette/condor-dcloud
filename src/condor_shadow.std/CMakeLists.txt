if(STD_UNIVERSE )

	add_definitions(-DSYSCALL_DEBUG)
	include_directories(${CONDOR_SOURCE_DIR}/src/condor_shadow.std)

	src_target_ref(condorsyscall "receivers;extern_path" CNDRSYSCL_REFS)

	condor_glob(Hdrs Srcs "")

	condor_exe(condor_shadow.std
	"${Hdrs};${Srcs};${CNDRSYSCL_REFS}" ${C_SBIN}
	"${CONDOR_TOOL_LIBS};syscall_numbers;qmgmt;ckpt_server_api;stdunivio;${KRB5_FOUND};${OPENSSL_FOUND};${PCRE_FOUND};${ZLIB_FOUND}")

	add_dependencies( condor_shadow.std condorsyscall )


endif(STD_UNIVERSE)
