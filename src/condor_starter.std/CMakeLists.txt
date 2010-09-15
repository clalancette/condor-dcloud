if(STD_UNIVERSE )

	# include the other libs.
	file( GLOB RmvElements test* driver.cpp)
 
	condor_glob(Hdrs Srcs "${RmvElements}")

	condor_exe(condor_starter.std
	"${Hdrs};${Srcs};${CKPT_REFS};${CNDRSYSCL_REFS}" ${C_SBIN}
	"${CONDOR_TOOL_LIBS};std_local_ref;ckpt_server_api;stdunivio;${KRB5_FOUND};${OPENSSL_FOUND};${PCRE_FOUND};${ZLIB_FOUND}")

endif(STD_UNIVERSE)
