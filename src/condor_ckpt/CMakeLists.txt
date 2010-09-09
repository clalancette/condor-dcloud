##############################################################
## Abandon all hope ye who enter here:
## this be the mystical workings of the infamous
## standard universe.  No moral platitudes apply, so
## batton your hatches, find yourself a cup of ale,
## because you are about to loose your mind.
## YARRR!
##############################################################
if( STD_UNIVERSE )

	###########################################################################
	# NOTE: all glibc ripping which existed here has moved to syscall_lib
	# where it is actually bound in.
	###########################################################################
	
	###########################################################################
	# define the actual targets, and copiler options
	###########################################################################
	add_definitions( ${STD_U_C_FLAGS} -DFILE_TABLE -DIN_CKPT_LIB)
	condor_selective_glob( "*file*;signals*;image*;machdep.LINUX.cpp;tmp_*;syscall_*;fake_*;condor_error*;eprintf*;shared_*;malloc-user*;gto*;maps*" CommonCkptSrcs )

	# this is kind of a red herring as it only works on 32-bit deb in current build
	if (DOES_COMPRESS_CKPT)
		add_definitions(-DCOMPRESS_CKPT -DMORECORE=condor_morecore -DHAVE_MMAP=0 -Dmalloc_getpagesize=8192)
		condor_selective_glob("malloc-condor.cpp;malloc.cpp" CompressSrcs)
	endif(DOES_COMPRESS_CKPT)

	# we create a local lib, but it's never actually used
	# instead condorsyscall will rip out the target refs.
	condor_static_lib( ckpt "${CommonCkptSrcs};${CompressSrcs}")

endif(STD_UNIVERSE)