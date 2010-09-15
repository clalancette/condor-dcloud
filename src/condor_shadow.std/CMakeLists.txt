if(STD_UNIVERSE )

	add_definitions(-DSYSCALL_DEBUG)
	include_directories(${CONDOR_SOURCE_DIR}/src/condor_shadow.std)

	condor_glob(Hdrs Srcs "")

	condor_exe(condor_shadow.std
	"${Hdrs};${Srcs};${CNDRSYSCL_REFS}" ${C_SBIN}
	"${CONDOR_TOOL_LIBS};std_local_ref;syscall_numbers;qmgmt;ckpt_server_api;stdunivio;${KRB5_FOUND};${OPENSSL_FOUND};${PCRE_FOUND};${ZLIB_FOUND}")

endif(STD_UNIVERSE)
