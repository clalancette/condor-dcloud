#
# In most modules, we compile with -fPIC.  Turn it off for this
# module, because it interferes with checkpointing.
#

if( LINUX AND NOT CLIPPED )

	condor_selective_glob( "*file*" FileTableSrcs )

	


	if (DOES_SAVE_SIGSTATE)
	endif(DOES_SAVE_SIGSTATE)

endif( LINUX AND NOT CLIPPED )