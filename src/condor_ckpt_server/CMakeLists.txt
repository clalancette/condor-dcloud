
condor_selective_glob("server_interface.*" ServerAPISrcs )

condor_static_lib( ckpt_server_api "network2.cpp;network2.h;${ServerAPISrcs}" )


if(STD_UNIVERSE)
	condor_selective_glob("server_file*" unusedSrcs )
	condor_glob( HeaderFiles SourceFiles "${ServerAPISrcs};${unusedSrcs}" )
	condor_exe( condor_ckpt_server "${HeaderFiles};${SourceFiles}" ${C_SBIN} "${CONDOR_TOOL_LIBS};${OPENSSL_FOUND};${KRB5_FOUND};${PCRE_FOUND}")
endif(STD_UNIVERSE)
