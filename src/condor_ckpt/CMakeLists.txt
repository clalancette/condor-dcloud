#############################################################
## Welcome to the standard universe, where nothing is "standard"
## and in fact it is anything but.
#############################################################
if( LINUX AND NOT CLIPPED AND GLIBC_VERSION AND NOT PROPER )

	include (StdUnivExtractNToUpper)
	#for safety sake disable all compilation flags, only enable warnings.
	#set (CMAKE_CXX_FLAGS "-Wall")

	###########################################################################
	# Begin prebuild section, symbol transform done up front when cmake is run
	# as this should be a 1-time pad
	#
	# Summary - standard universe is extracting specific standard .o files which
	# it then renames the symbols and inserts it's own proxy/symbols in their place.
	###########################################################################
	# copy in crt1 && redefine main.
	exec_program ( gcc ARGS -print-file-name=crt1.o OUTPUT_VARIABLE CRT1 )
	exec_program ( cp ARGS ${CRT1} . )
	exec_program ( objcopy ARGS --redefine-sym main=MAIN ${_OBJ_FILE} )

	# calls to stdu_extract_and_toupper will append to a global var STDU_OBJS to be used later
	stdu_extract_and_toupper(${EXT_GLIBC_FOUND} syscall.o "syscall")
	stdu_extract_and_toupper(${EXT_GLIBC_FOUND} pipe.o "pipe;__pipe")
	stdu_extract_and_toupper(${EXT_GLIBC_FOUND} mmap.o "mmap;__mmap")
	stdu_extract_and_toupper(${EXT_GLIBC_FOUND} sigsuspend.o "sigsuspend;__sigsuspend")
	stdu_extract_and_toupper(${EXT_GLIBC_FOUND} sigaction.o "sigaction;__sigaction")
	if (NOT GLIBC25 AND NOT GLIBC27)
		stdu_extract_and_toupper(${EXT_GLIBC_FOUND} sysdep.o "__syscall_error")
	endif
	
	###########################################################################
	# now define the actual targets, and copiler options
	###########################################################################
	add_definitions(-DFILE_TABLE -DIN_CKPT_LIB)
	condor_selective_glob( "*file*;signals*;image*;machdep.LINUX.cpp;tmp_*;syscall_*;fake_*;condor_error*;eprintf*;shared_*;malloc-user*;gto*;maps*" CommonCkptSrcs )

	if (DOES_SAVE_SIGSTATE)
		add_definitions(-DSAVE_SIGSTATE)
	endif(DOES_SAVE_SIGSTATE)

	# this is kind of a red herring as it only works on 32-bit deb in current build
	if (DOES_COMPRESS_CKPT)
		add_definitions(-DCOMPRESS_CKPT -DMORECORE=condor_morecore -DHAVE_MMAP=0 -Dmalloc_getpagesize=8192)
		condor_selective_glob("malloc-condor.cpp;malloc.cpp" CompressSrcs)
	endif(DOES_COMPRESS_CKPT)

	# now create the condorckpt object.
	condor_static_lib(condorckpt "${STDU_OBJS};${CRT1};${CommonCkptSrcs};${CompressSrcs}")

endif(LINUX AND NOT CLIPPED AND GLIBC_VERSION AND NOT PROPER )