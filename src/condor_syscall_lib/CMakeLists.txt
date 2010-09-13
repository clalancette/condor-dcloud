##############################################################
## Abandon all hope ye who enter here:
## this be the mystical workings of the infamous
## standard universe.  No moral platitudes apply, so
## batton your hatches, find yourself a cup of ale,
## because you are about to loose your mind.
## YARRR!
##############################################################
if( STD_UNIVERSE )

	include (StdUnivExtractNToUpper)
	add_definitions(${STD_U_C_FLAGS} -DREMOTE_SYSCALLS -DSKIP_AUTHENTICATION -DHEADER_DES_LOCL_H -DFILE_TABLE )
	include_directories(${CONDOR_SOURCE_DIR}/src/condor_shadow.std ${CONDOR_SOURCE_DIR}/src/condor_ckpt)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")

	###########################################################################
	# Begin symbol transformation.
	#
	# Summary - standard universe is extracting specific standard .o files which
	# it then renames the symbols and inserts it's own proxy/symbols in their place.
	# which can call the originals.
	###########################################################################
	exec_program ( gcc ARGS -print-file-name=crt1.o OUTPUT_VARIABLE CRT1 )
	exec_program ( objcopy ARGS --redefine-sym main=MAIN ${CRT1} ${CMAKE_CURRENT_SOURCE_DIR}/condor_rt0 )

	##################################################
	# rip from custom built glibc and uppercase the defined symbols
	stdu_extract_and_toupper(${EXT_GLIBC_FOUND} getcwd.o "getcwd;__getcwd")
	stdu_extract_and_toupper(${EXT_GLIBC_FOUND} mmap.o "mmap;__mmap")
	stdu_extract_and_toupper(${EXT_GLIBC_FOUND} syscall.o "syscall")
	stdu_extract_and_toupper(${EXT_GLIBC_FOUND} geteuid.o "geteuid;__geteuid")
	stdu_extract_and_toupper(${EXT_GLIBC_FOUND} getegid.o "getegid;__getegid")
	stdu_extract_and_toupper(${EXT_GLIBC_FOUND} getppid.o "getppid;__getppid")
	stdu_extract_and_toupper(${EXT_GLIBC_FOUND} fork.o "fork;__fork")
	stdu_extract_and_toupper(${EXT_GLIBC_FOUND} pipe.o "pipe;__pipe")
	stdu_extract_and_toupper(${EXT_GLIBC_FOUND} getlogin.o "getlogin")
	stdu_extract_and_toupper(${EXT_GLIBC_FOUND} sysdep.o "__syscall_error;__syscall_error_1")
	stdu_extract_and_toupper(${EXT_GLIBC_FOUND} sigsuspend.o "sigsuspend;__sigsuspend")
	stdu_extract_and_toupper(${EXT_GLIBC_FOUND} truncate.o "truncate;__truncate")
	stdu_extract_and_toupper(${EXT_GLIBC_FOUND} sigaction.o "sigaction;__sigaction")
	stdu_extract_and_toupper(${EXT_GLIBC_FOUND} getdirentries.o "getdirentries")

	##################################################
	# part 1:
	# now the below section is some clever magic to
	# elim the need of shuffling objs instead find
	# matching refs in other libs and ensure the are built
	# prior this target running.
	##################################################
	src_target_ref( ckpt "^.*$" CKPT_REFS)
	src_target_ref( utils "flock;dprintf_common;except;update_rusage;internet;mkargv;filename_tools;strupr;mark_thread;condor_version;condor_ver_info;stat_wrapper;stat_wrapper_internal;condor_open;selector;MyString;subsystem_info;HashTable" UTIL_REFS)
	src_target_ref( cedar "stream;sig_num;open_flags;fcntl_cmd;errno_num;condor_rw" IO_REFS)
	src_target_ref( stdunivio "std_univ_sock;std_univ_buffers" STDIO_REF )

	##################################################
	# begin all the stub-gen targets
	get_target_property(STUB_GEN stub_gen LOCATION)
	dump_local_flags(COMP_DEFS)
	command_target (switches ${CMAKE_C_COMPILER} "-E;-x;c-header;-DSWITCH;${COMP_DEFS};syscall.tmpl;|;${STUB_GEN};-mode;switches;-p;switches.prologue;-e;switches.epilogue;>;switches.cpp" switches.cpp)
	command_target (senders ${CMAKE_C_COMPILER} "-E;-x;c-header;-DSENDER;${COMP_DEFS};syscall.tmpl;|;${STUB_GEN};-mode;senders;-p;senders.prologue;-e;senders.epilogue;>;senders.cpp" senders.cpp)
	command_target (receivers ${CMAKE_C_COMPILER} "-E;-x;c-header;-DRECEIVER;${COMP_DEFS};syscall.tmpl;|;${STUB_GEN};-mode;receivers;-p;receivers.prologue;-e;receivers.epilogue;-n; syscheck;>;receivers.cpp" receivers.cpp)
	command_target (sys-list ${CMAKE_CXX_COMPILER} "-E;-x;c-header;-DSWITCH;${COMP_DEFS};syscall.tmpl;|;${STUB_GEN};-mode;listcalls;>;syscall-list" syscall-list)

	##################################################
	# finally add in the local sources, remove stale stubs if they exist so we don't have duplicates
	file( GLOB StubRmvSrcs switches.cpp senders.cpp receivers.cpp )
	condor_glob( SyscallHdrs SyscallSrcs "${StubRmvSrcs}" )

	##################################################
	# now create the glorious syscall lib various sources.
	condor_static_lib( condorsyscall "senders.cpp;receivers.cpp;switches.cpp;${SyscallHdrs};${SyscallSrcs};${STDU_OBJS};${CKPT_REFS};${UTIL_REFS};${IO_REFS};${STDIO_REF}")

	##################################################
	# part 2:
	# the dependencies will ensure build dep order
	# it will also ensure that the .o's are up to date
	# and built with it's local flags
	##################################################
	add_dependencies( condorsyscall ckpt utils cedar stdunivio stub_gen )

	#################################################
	# here we begin the install target section 
	install ( FILES condor_rt0 DESTINATION ${C_LIB} )
	

endif( STD_UNIVERSE )
