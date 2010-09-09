
condor_selective_glob("server_interface.*" ServerAPISrcs )

condor_static_lib( ckpt_server_api "network2.cpp;network2.h;${ServerAPISrcs}" )


if(STD_UNIVERSE)
	condor_glob( HeaderFiles SourceFiles "${ServerAPISrcs}" )
	condor_exe( condor_ckpt_server "${HeaderFiles};${SourceFiles}" ${C_SBIN} "${TOOL_LINK_LIBS}")
endif(STD_UNIVERSE)