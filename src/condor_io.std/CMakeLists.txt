if( STD_UNIVERSE )

	add_definitions( -DSKIP_AUTHENTICATION -DHEADER_DES_LOCL_H )
	condor_glob( stdiohdrs stdiosrcs "" )
	condor_static_lib(stdunivio "${stdiohdrs};${stdiosrcs}")

endif( STD_UNIVERSE )