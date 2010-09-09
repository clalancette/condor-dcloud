
message(STATUS "***********************************************************")
message(STATUS "System: ${OS_NAME}(${OS_VER}) Arch=${SYS_ARCH} BitMode=${BIT_MODE}")
message(STATUS "********* BEGINNING CONFIGURATION *********")

##################################################
##################################################
include (FindThreads)
include (GlibcDetect)

add_definitions(-D${OS_NAME}=${OS_NAME}_${OS_VER})
add_definitions(-D${SYS_ARCH}=${SYS_ARCH})
add_definitions(-DPLATFORM=${CMAKE_SYSTEM})

set( CONDOR_EXTERNAL_DIR ${CONDOR_SOURCE_DIR}/externals )
set( CMAKE_VERBOSE_MAKEFILE TRUE )
set( BUILD_SHARED_LIBS FALSE )

# Windows is so different perform the check 1st and start setting the vars.
if(${OS_NAME} MATCHES "WIN" AND NOT ${OS_NAME} MATCHES "DARWIN")

	set(WINDOWS ON)
	add_definitions(-DWINDOWS)
	# The following is necessary for sdk/ddk version to compile against.
	# lowest common denominator is winxp (for now)
	add_definitions(-D_WIN32_WINNT=_WIN32_WINNT_WINXP)
	add_definitions(-DWINVER=_WIN32_WINNT_WINXP)
	add_definitions(-DNTDDI_VERSION=NTDDI_WINXP)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	set(CMD_TERM \r\n)
	set(C_WIN_BIN ${CONDOR_SOURCE_DIR}/msconfig) #${CONDOR_SOURCE_DIR}/build/backstage/win)
	set(BISON_SIMPLE ${C_WIN_BIN}/bison.simple)
	set(CMAKE_SUPPRESS_REGENERATION TRUE)

	set (HAVE_SNPRINTF 1)
	set (HAVE_WORKING_SNPRINTF 1)

else()

	set( CMD_TERM && )
	set( CMAKE_BUILD_TYPE RelWithDebInfo ) # = -O2 -g (package will strip the info)
	set( CMAKE_SUPPRESS_REGENERATION FALSE )

	# when we want to distro dynamic libraries only with localized rpaths.
	# set (CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
	# set (CMAKE_INSTALL_RPATH YOUR_LOC)
	# set (CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

	set(HAVE_PTHREAD_H ${CMAKE_HAVE_PTHREAD_H})

	find_library(HAVE_X11 X11)
	find_library(HAVE_DMTCP dmtcpaware HINTS /usr/local/lib/dmtcp )
	check_library_exists(dl dlopen "" HAVE_DLOPEN)
	check_symbol_exists(res_init "sys/types.h;netinet/in.h;arpa/nameser.h;resolv.h" HAVE_DECL_RES_INIT)

	check_function_exists("access" HAVE_ACCESS)
	check_function_exists("clone" HAVE_CLONE)
	check_function_exists("dirfd" HAVE_DIRFD)
	check_function_exists("execl" HAVE_EXECL)
	check_function_exists("fstat64" HAVE_FSTAT64)
	check_function_exists("_fstati64" HAVE__FSTATI64)
	check_function_exists("getdtablesize" HAVE_GETDTABLESIZE)
	check_function_exists("getpagesize" HAVE_GETPAGESIZE)
	check_function_exists("getwd" HAVE_GETWD)
	check_function_exists("inet_ntoa" HAS_INET_NTOA)
	check_function_exists("lchown" HAVE_LCHOWN)
	check_function_exists("lstat" HAVE_LSTAT)
	check_function_exists("lstat64" HAVE_LSTAT64)
	check_function_exists("_lstati64" HAVE__LSTATI64)
	check_function_exists("mkstemp" HAVE_MKSTEMP)
	check_function_exists("setegid" HAVE_SETEGID)
	check_function_exists("setenv" HAVE_SETENV)
	check_function_exists("seteuid" HAVE_SETEUID)
	check_function_exists("setlinebuf" HAVE_SETLINEBUF)
	check_function_exists("snprintf" HAVE_SNPRINTF)
	check_function_exists("snprintf" HAVE_WORKING_SNPRINTF)	

	check_function_exists("stat64" HAVE_STAT64)
	check_function_exists("_stati64" HAVE__STATI64)
	check_function_exists("statfs" HAVE_STATFS)
	check_function_exists("statvfs" HAVE_STATVFS)
	check_function_exists("res_init" HAVE_DECL_RES_INIT)
	check_function_exists("strcasestr" HAVE_STRCASESTR)
	check_function_exists("strsignal" HAVE_STRSIGNAL)
	check_function_exists("unsetenv" HAVE_UNSETENV)
	check_function_exists("vasprintf" HAVE_VASPRINTF)
	
	# we can likely put many of the checks below in here.
	check_include_files("dlfcn.h" HAVE_DLFCN_H)
	check_include_files("inttypes.h" HAVE_INTTYPES_H)
	check_include_files("ldap.h" HAVE_LDAP_H)
	check_include_files("net/if.h" HAVE_NET_IF_H)
	check_include_files("os_types.h" HAVE_OS_TYPES_H)
	check_include_files("resolv.h" HAVE_RESOLV_H)
	check_include_files("sys/mount.h" HAVE_SYS_MOUNT_H)
	check_include_files("sys/param.h" HAVE_SYS_PARAM_H)
	check_include_files("sys/personality.h" HAVE_SYS_PERSONALITY_H)
	check_include_files("sys/syscall.h" HAVE_SYS_SYSCALL_H)
	check_include_files("sys/statfs.h" HAVE_SYS_STATFS_H)
	check_include_files("sys/types.h" HAVE_SYS_TYPES_H)
	check_include_files("sys/vfs.h" HAVE_SYS_VFS_H)
	check_include_files("stdint.h" HAVE_STDINT_H)
	check_include_files("ustat.h" HAVE_USTAT_H)
	check_include_files("valgrind.h" HAVE_VALGRIND_H)
	check_include_files("procfs.h" HAVE_PROCFS_H)
	check_include_files("sys/procfs.h" HAVE_SYS_PROCFS_H)

	check_type_exists("struct ifconf" "net/if.h" HAVE_STRUCT_IFCONF)
	check_type_exists("struct ifreq" "net/if.h" HAVE_STRUCT_IFREQ)

	check_struct_has_member("struct statfs" f_fstyp "sys/statfs.h" HAVE_STRUCT_STATFS_F_FSTYP)
	check_struct_has_member("struct statfs" f_fstypename "sys/statfs.h" HAVE_STRUCT_STATFS_F_FSTYPENAME)
	check_struct_has_member("struct statfs" f_type "sys/statfs.h" HAVE_STRUCT_STATFS_F_TYPE)
	check_struct_has_member("struct statvfs" f_basetype "sys/types.h;sys/statvfs.h" HAVE_STRUCT_STATVFS_F_BASETYPE)

	# the follow arg checks should be a posix check.
	# previously they were ~=check_cxx_source_compiles
	set(STATFS_ARGS "2")
	set(SIGWAIT_ARGS "2")

	# note the following is fairly gcc specific, but *we* only check gcc version in std:u which it requires.
	exec_program (${CMAKE_CXX_COMPILER}
    		ARGS ${CMAKE_CXX_COMPILER_ARG1} -dumpversion
    		OUTPUT_VARIABLE CMAKE_CXX_COMPILER_VERSION )

	exec_program (${CMAKE_C_COMPILER}
    		ARGS ${CMAKE_C_COMPILER_ARG1} -dumpversion
    		OUTPUT_VARIABLE CMAKE_C_COMPILER_VERSION )

	option(PROPER "Try to build using native env" ON)

endif()

find_program(HAVE_VMWARE vmware)
check_type_size("id_t" HAVE_ID_T)
check_type_size("__int64" HAVE___INT64)
check_type_size("int64_t" HAVE_INT64_T)
check_type_size("long long" HAVE_LONG_LONG)

##################################################
##################################################
# Now checking *nix OS based options 
set(HAS_FLOCK ON)
set(DOES_SAVE_SIGSTATE OFF)

if (${OS_NAME} STREQUAL "SUNOS")
	set(SOLARIS ON)
	set(NEEDS_64BIT_SYSCALLS ON)
	set(NEEDS_64BIT_STRUCTS ON)
	set(DOES_SAVE_SIGSTATE ON)
	set(HAS_FLOCK OFF)
	add_definitions(-DSolaris)
	if (${OS_VER} STREQUAL "5.9")
		add_definitions(-DSolaris29)
	elseif(${OS_VER} STREQUAL "5.10")
		add_definitions(-DSolaris210)
	else()
		message(FATAL_ERROR "unknown version of Solaris")
	endif()
	add_definitions(-D_STRUCTURED_PROC)
	set(HAS_INET_NTOA ON)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lkstat -lelf -lsocket")
elseif(${OS_NAME} STREQUAL "LINUX")

	set(LINUX ON)
	set(DOES_SAVE_SIGSTATE ON)
	check_symbol_exists(SIOCETHTOOL "linux/sockios.h" HAVE_DECL_SIOCETHTOOL)
	check_symbol_exists(SIOCGIFCONF "linux/sockios.h" HAVE_DECL_SIOCGIFCONF)
	check_include_files("linux/ethtool.h" HAVE_LINUX_ETHTOOL_H)
	check_include_files("linux/magic.h" HAVE_LINUX_MAGIC_H)
	check_include_files("linux/nfsd/const.h" HAVE_LINUX_NFSD_CONST_H)
	check_include_files("linux/personality.h" HAVE_LINUX_PERSONALITY_H)
	check_include_files("linux/sockios.h" HAVE_LINUX_SOCKIOS_H)
	check_include_files("linux/types.h" HAVE_LINUX_TYPES_H)

	dprint("Threaded functionality only enable in Linux and Windows")
	set(HAS_PTHREADS ${CMAKE_USE_PTHREADS_INIT})
	set(HAVE_PTHREADS ${CMAKE_USE_PTHREADS_INIT})

	#The following checks are for std:u only.
	glibc_detect( GLIBC_VERSION )

elseif(${OS_NAME} STREQUAL "AIX")
	set(AIX ON)
	set(DOES_SAVE_SIGSTATE ON)
	set(NEEDS_64BIT_STRUCTS ON)
elseif(${OS_NAME} STREQUAL "DARWIN")
	add_definitions(-DDarwin)
	set(MACOS ON)
elseif(${OS_NAME} STREQUAL "HPUX")
	set(HPUX ON)
	set(DOES_SAVE_SIGSTATE ON)
	set(NEEDS_64BIT_STRUCTS ON)
endif()

##################################################
##################################################
# compilation/build options.
option(ENABLE_CHECKSUM_SHA1 "Enable production and validation of SHA1 checksums." OFF)
option(ENABLE_CHECKSUM_MD5 "Enable production and validation of MD5 checksums for released packages." ON)
option(HAVE_HIBERNATION "Support for condor controlled hibernation" ON)
option(WANT_LEASE_MANAGER "Enable lease manager functionality" ON)
option(WANT_QUILL "Enable quill functionality" OFF)
option(HAVE_JOB_HOOKS "Enable job hook functionality" ON)
option(NEEDS_KBDD "Enable KBDD functionality" ON)
option(HAVE_BACKFILL "Compiling support for any backfill system" ON)
option(HAVE_BOINC "Compiling support for backfill with BOINC" ON)
option(SOFT_IS_HARD "Enable strict checking for WITH_<LIB>" OFF)
option(CLIPPED "enable/disable the standard universe" ON)

if (NOT HPUX)
	option(HAVE_SHARED_PORT "Support for condor_shared_port" ON)
	if (NOT WINDOWS)
		set (HAVE_SCM_RIGHTS_PASSFD ON)
	endif()
endif(NOT HPUX)

if (NOT WINDOWS)
	option(BUILD_TESTS "Will build internal test applications" ON)
	option(HAVE_SSH_TO_JOB "Support for condor_ssh_to_job" ON)
	option(PROPER "Try to build using native env" ON)
else()
	option(PROPER "Try to build using native env" OFF)
endif()

if (BUILD_TESTS)
	set(TEST_TARGET_DIR ${CONDOR_SOURCE_DIR}/src/condor_tests)
endif(BUILD_TESTS)

##################################################
##################################################
# setup for the externals, the variables defined here
# are used in the construction of externals within
# the condor build.  The point of main interest is
# how "cacheing" is performed by performing the build
# external to the tree itself.
if (PROPER)
	message(STATUS "********* Configuring externals using [local env] a.k.a. PROPER *********")
	find_path(HAVE_OPENSSL_SSL_H "openssl/ssl.h")
	find_path(HAVE_PCRE_H "pcre.h")
	find_path(HAVE_PCRE_PCRE_H "pcre/pcre.h" )
else(PROPER)
	message(STATUS "********* Configuring externals using [uw-externals] a.k.a NONPROPER *********")
	option(SCRATCH_EXTERNALS "Will put the externals into scratch location" ON)
endif(PROPER)

if (SCRATCH_EXTERNALS)
	if (WINDOWS)
		set (EXTERNAL_STAGE C:/temp/scratch/externals/cmake/${PACKAGE_NAME}_${PACKAGE_VERSION}/root)
		set (EXTERNAL_DL C:/temp/scratch/externals/cmake/${PACKAGE_NAME}_${PACKAGE_VERSION}/download)
	else(WINDOWS)
		set (EXTERNAL_STAGE /scratch/externals/cmake/${PACKAGE_NAME}_${PACKAGE_VERSION}/stage/root)
		set (EXTERNAL_DL /scratch/externals/cmake/${PACKAGE_NAME}_${PACKAGE_VERSION}/externals/stage/download)

		set (EXTERNAL_MOD_DEP /scratch/externals/cmake/${PACKAGE_NAME}_${PACKAGE_VERSION}/mod.txt)
		add_custom_command(
		OUTPUT ${EXTERNAL_MOD_DEP}
		COMMAND chmod
		ARGS -f -R a+rwX /scratch/externals/cmake && touch ${EXTERNAL_MOD_DEP}
		COMMENT "changing ownership on externals cache because so on multiple user machines they can take advantage" )
	endif(WINDOWS)
else(SCRATCH_EXTERNALS)
	set (EXTERNAL_STAGE ${CONDOR_EXTERNAL_DIR}/stage/root)
	set (EXTERNAL_DL ${CONDOR_EXTERNAL_DIR}/stage/download)
endif(SCRATCH_EXTERNALS)

dprint("EXTERNAL_STAGE=${EXTERNAL_STAGE}")
set (EXTERNAL_BUILD_PREFIX ${EXTERNAL_STAGE}/opt)

# let cmake carve out the paths for the externals
file (MAKE_DIRECTORY ${EXTERNAL_DL}
	${EXTERNAL_STAGE}/include
	${EXTERNAL_STAGE}/lib
	${EXTERNAL_STAGE}/lib64
	${EXTERNAL_STAGE}/opt )

include_directories( ${EXTERNAL_STAGE}/include )
link_directories( ${EXTERNAL_STAGE}/lib ${EXTERNAL_STAGE}/lib64 )

###########################################
add_subdirectory(${CONDOR_EXTERNAL_DIR}/bundles/krb5/1.4.3-p0)
add_subdirectory(${CONDOR_EXTERNAL_DIR}/bundles/openssl/0.9.8h-p2)
add_subdirectory(${CONDOR_EXTERNAL_DIR}/bundles/pcre/7.6)
add_subdirectory(${CONDOR_EXTERNAL_DIR}/bundles/gsoap/2.7.10-p5)
add_subdirectory(${CONDOR_SOURCE_DIR}/src/classad)

if (NOT WIN_EXEC_NODE_ONLY)
	add_subdirectory(${CONDOR_EXTERNAL_DIR}/bundles/hadoop/0.20.0-p2)
	add_subdirectory(${CONDOR_EXTERNAL_DIR}/bundles/postgresql/8.2.3-p1)
	add_subdirectory(${CONDOR_EXTERNAL_DIR}/bundles/drmaa/1.6)
endif(NOT WIN_EXEC_NODE_ONLY)

if (NOT WINDOWS)
	add_subdirectory(${CONDOR_EXTERNAL_DIR}/bundles/zlib/1.2.3)
	add_subdirectory(${CONDOR_EXTERNAL_DIR}/bundles/curl/7.19.6-p1 )
	add_subdirectory(${CONDOR_EXTERNAL_DIR}/bundles/coredumper/0.2)
	add_subdirectory(${CONDOR_EXTERNAL_DIR}/bundles/unicoregahp/1.2.0)
	add_subdirectory(${CONDOR_EXTERNAL_DIR}/bundles/expat/2.0.1)
	add_subdirectory(${CONDOR_EXTERNAL_DIR}/bundles/gcb/1.5.6)
	add_subdirectory(${CONDOR_EXTERNAL_DIR}/bundles/libxml2/2.7.3)
	add_subdirectory(${CONDOR_EXTERNAL_DIR}/bundles/libvirt/0.6.2)

	# globus is an odd *beast* which requires a bit more config.
	add_subdirectory(${CONDOR_EXTERNAL_DIR}/bundles/globus/5.0.1)
	if (PROPER AND GLOBUS_FOUND)
		# locs for current rpm.
		include_directories(/usr/include/globus /usr/lib64/globus/include/)
	else(PROPER AND GLOBUS_FOUND)
		include_directories(${EXTERNAL_STAGE}/include/${GLOBUS_FLAVOR})
	endif(PROPER AND GLOBUS_FOUND)

	add_subdirectory(${CONDOR_EXTERNAL_DIR}/bundles/blahp/1.16.0)
	add_subdirectory(${CONDOR_EXTERNAL_DIR}/bundles/voms/1.8.8_2-p2)
	add_subdirectory(${CONDOR_EXTERNAL_DIR}/bundles/srb/3.2.1-p2)
	#add_subdirectory(${CONDOR_SOURCE_DIR}/bundles/cream/1.10.1-p2)

	# the following logic if for standard universe *only*
	if (LINUX AND NOT CLIPPED AND GLIBC_VERSION AND NOT PROPER)

		add_subdirectory(${CONDOR_EXTERNAL_DIR}/bundles/glibc)

		if (${BIT_MODE} STREQUAL "32")
			set (DOES_COMPRESS_CKPT ON) # this is total crap
		endif(${BIT_MODE} STREQUAL "32")

		if (DOES_SAVE_SIGSTATE)
			append_var(STD_U_C_FLAGS -DSAVE_SIGSTATE)
		endif(DOES_SAVE_SIGSTATE)

		set (STD_UNIVERSE ON)
		message( STATUS "** Standard Universe Enabled **")
	else()
		message( STATUS "** Standard Universe Disabled **")
	endif()

endif(NOT WINDOWS)

if (CONDOR_EXTERNALS AND NOT WINDOWS)
	### addition of a single externals target which allows you to 
	add_custom_target( externals DEPENDS ${EXTERNAL_MOD_DEP} )
	add_dependencies( externals ${CONDOR_EXTERNALS} )
endif(CONDOR_EXTERNALS AND NOT WINDOWS)

message(STATUS "********* External configuration complete (dropping config.h) *********")
dprint("CONDOR_EXTERNALS=${CONDOR_EXTERNALS}")

########################################################
configure_file(${CONDOR_SOURCE_DIR}/src/condor_includes/config.h.cmake
${CONDOR_SOURCE_DIR}/src/condor_includes/config.h)
add_definitions(-DHAVE_CONFIG_H)

###########################################
# include and link locations
include_directories(${EXTERNAL_STAGE}/include)
link_directories(${EXTERNAL_STAGE}/lib)

if ( $ENV{JAVA_HOME} )
	include_directories($ENV{JAVA_HOME}/include)
endif()
include_directories(${CONDOR_SOURCE_DIR}/src/condor_includes)
include_directories(${CONDOR_SOURCE_DIR}/src/condor_utils)
set (DAEMON_CORE ${CONDOR_SOURCE_DIR}/src/condor_daemon_core.V6) #referenced elsewhere primarily for soap gen stuff 
include_directories(${DAEMON_CORE})
include_directories(${CONDOR_SOURCE_DIR}/src/condor_daemon_client)
include_directories(${CONDOR_SOURCE_DIR}/src/ccb)
include_directories(${CONDOR_SOURCE_DIR}/src/condor_io)
include_directories(${CONDOR_SOURCE_DIR}/src/h)
include_directories(${CONDOR_SOURCE_DIR}/src/classad)
###########################################

###########################################
#extra build flags and link libs.
if (HAVE_EXT_OPENSSL)
	add_definitions(-DWITH_OPENSSL) # used only by SOAP
endif(HAVE_EXT_OPENSSL)

###########################################
# order of the below elements is important, do not touch unless you know what you are doing.
# otherwise you will break due to stub collisions.
set (CONDOR_LIBS "procd_client;daemon_core;daemon_client;procapi;cedar;privsep;${CLASSADS_FOUND};sysapi;ccb;utils;${GLOBUS_FOUND};${GCB_FOUND}")
set (CONDOR_TOOL_LIBS "procd_client;daemon_client;procapi;cedar;privsep;${CLASSADS_FOUND};sysapi;ccb;utils;${GLOBUS_FOUND};${GCB_FOUND}")

message(STATUS "----- Begin compiler options/flags check -----")

if (CONDOR_CXX_FLAGS)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CONDOR_CXX_FLAGS}")
endif()

if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251 /wd4275 /wd4996 /wd4273")
	set(CONDOR_WIN_LIBS "crypt32.lib;mpr.lib;psapi.lib;mswsock.lib;netapi32.lib;imagehlp.lib;ws2_32.lib;powrprof.lib;iphlpapi.lib;userenv.lib;Pdh.lib")
else(MSVC)

	if (GLIBC_VERSION)
		add_definitions(-DGLIBC=GLIBC)
		add_definitions(-DGLIBC${GLIBC_VERSION}=GLIBC${GLIBC_VERSION})
		set(GLIBC${GLIBC_VERSION} ON)
	endif(GLIBC_VERSION)

	check_cxx_compiler_flag(-Wall cxx_Wall)
	if (cxx_Wall)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
	endif(cxx_Wall)

	check_cxx_compiler_flag(-W cxx_W)
	if (cxx_W)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W")
	endif(cxx_W)

	check_cxx_compiler_flag(-Wextra cxx_Wextra)
	if (cxx_Wextra)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
	endif(cxx_Wextra)

	check_cxx_compiler_flag(-Wfloat-equal cxx_Wfloat_equal)
	if (cxx_Wfloat_equal)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wfloat-equal")
	endif(cxx_Wfloat_equal)

	check_cxx_compiler_flag(-Wshadow cxx_Wshadow)
	if (cxx_Wshadow)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wshadow")
	endif(cxx_Wshadow)

	# someone else can enable this, as it overshadows all other warnings and can be wrong.
	# check_cxx_compiler_flag(-Wunreachable-code cxx_Wunreachable_code)
	# if (cxx_Wunreachable_code)
	#	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wunreachable-code")
	# endif(cxx_Wunreachable_code)

	check_cxx_compiler_flag(-Wendif-labels cxx_Wendif_labels)
	if (cxx_Wendif_labels)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wendif-labels")
	endif(cxx_Wendif_labels)

	check_cxx_compiler_flag(-Wpointer-arith cxx_Wpointer_arith)
	if (cxx_Wpointer_arith)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpointer-arith")
	endif(cxx_Wpointer_arith)

	check_cxx_compiler_flag(-Wcast-qual cxx_Wcast_qual)
	if (cxx_Wcast_qual)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wcast-qual")
	endif(cxx_Wcast_qual)

	check_cxx_compiler_flag(-Wcast-align cxx_Wcast_align)
	if (cxx_Wcast_align)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wcast-align")
	endif(cxx_Wcast_align)

	check_cxx_compiler_flag(-Wvolatile-register-var cxx_Wvolatile_register_var)
	if (cxx_Wvolatile_register_var)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wvolatile-register-var")
	endif(cxx_Wvolatile_register_var)

	check_cxx_compiler_flag(-fstack-protector cxx_fstack_protector)
	if (cxx_fstack_protector)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector")
	endif(cxx_fstack_protector)

	check_cxx_compiler_flag(-fnostack-protector cxx_fnostack_protector)
	if (cxx_fnostack_protector)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fnostack-protector")
	endif(cxx_fnostack_protector)

	check_cxx_compiler_flag(-rdynamic cxx_rdynamic)
	if (cxx_rdynamic)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic")
	endif(cxx_rdynamic)

	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--warn-once -Wl,--warn-common")

	if(HAVE_DLOPEN)
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -ldl")
	endif(HAVE_DLOPEN)

	if (NOT PROPER)
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lresolv -lcrypt")
	endif(NOT PROPER)

	if (HAVE_PTHREADS)
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
	endif(HAVE_PTHREADS)

	check_cxx_compiler_flag(-shared HAVE_CC_SHARED)

endif(MSVC)

message(STATUS "----- End compiler options/flags check -----")
dprint("----- Begin CMake Var DUMP -----")
# if you are building in-source, this is the same as CMAKE_SOURCE_DIR, otherwise 
# this is the top level directory of your build tree 
dprint ( "CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}" )

# if you are building in-source, this is the same as CMAKE_CURRENT_SOURCE_DIR, otherwise this 
# is the directory where the compiled or generated files from the current CMakeLists.txt will go to 
dprint ( "CMAKE_CURRENT_BINARY_DIR: ${CMAKE_CURRENT_BINARY_DIR}" )

# this is the directory, from which cmake was started, i.e. the top level source directory 
dprint ( "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}" )

# this is the directory where the currently processed CMakeLists.txt is located in 
dprint ( "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}" )

# contains the full path to the top level directory of your build tree 
dprint ( "PROJECT_BINARY_DIR: ${PROJECT_BINARY_DIR}" )

# contains the full path to the root of your project source directory,
# i.e. to the nearest directory where CMakeLists.txt contains the PROJECT() command 
dprint ( "PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}" )

# set this variable to specify a common place where CMake should put all executable files
# (instead of CMAKE_CURRENT_BINARY_DIR)
dprint ( "EXECUTABLE_OUTPUT_PATH: ${EXECUTABLE_OUTPUT_PATH}" )

# set this variable to specify a common place where CMake should put all libraries 
# (instead of CMAKE_CURRENT_BINARY_DIR)
dprint ( "LIBRARY_OUTPUT_PATH: ${LIBRARY_OUTPUT_PATH}" )

# tell CMake to search first in directories listed in CMAKE_MODULE_PATH
# when you use FIND_PACKAGE() or INCLUDE()
dprint ( "CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}" )

# this is the complete path of the cmake which runs currently (e.g. /usr/local/bin/cmake) 
dprint ( "CMAKE_COMMAND: ${CMAKE_COMMAND}" )

# this is the CMake installation directory 
dprint ( "CMAKE_ROOT: ${CMAKE_ROOT}" )

# this is the filename including the complete path of the file where this variable is used. 
dprint ( "CMAKE_CURRENT_LIST_FILE: ${CMAKE_CURRENT_LIST_FILE}" )

# this is linenumber where the variable is used
dprint ( "CMAKE_CURRENT_LIST_LINE: ${CMAKE_CURRENT_LIST_LINE}" )

# this is used when searching for include files e.g. using the FIND_PATH() command.
dprint ( "CMAKE_INCLUDE_PATH: ${CMAKE_INCLUDE_PATH}" )

# this is used when searching for libraries e.g. using the FIND_LIBRARY() command.
dprint ( "CMAKE_LIBRARY_PATH: ${CMAKE_LIBRARY_PATH}" )

# the complete system name, e.g. "Linux-2.4.22", "FreeBSD-5.4-RELEASE" or "Windows 5.1" 
dprint ( "CMAKE_SYSTEM: ${CMAKE_SYSTEM}" )

# the short system name, e.g. "Linux", "FreeBSD" or "Windows"
dprint ( "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}" )

# only the version part of CMAKE_SYSTEM 
dprint ( "CMAKE_SYSTEM_VERSION: ${CMAKE_SYSTEM_VERSION}" )

# the processor name (e.g. "Intel(R) Pentium(R) M processor 2.00GHz") 
dprint ( "CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}" )

# is TRUE on all UNIX-like OS's, including Apple OS X and CygWin
dprint ( "UNIX: ${UNIX}" )

# is TRUE on Windows, including CygWin 
dprint ( "WIN32: ${WIN32}" )

# is TRUE on Apple OS X
dprint ( "APPLE: ${APPLE}" )

# is TRUE when using the MinGW compiler in Windows
dprint ( "MINGW: ${MINGW}" )

# is TRUE on Windows when using the CygWin version of cmake
dprint ( "CYGWIN: ${CYGWIN}" )

# is TRUE on Windows when using a Borland compiler 
dprint ( "BORLAND: ${BORLAND}" )

if (WINDOWS)
	dprint ( "MSVC: ${MSVC}" )
	dprint ( "MSVC_IDE: ${MSVC_IDE}" )
	dprint ( "MSVC60: ${MSVC60}" )
	dprint ( "MSVC70: ${MSVC70}" )
	dprint ( "MSVC71: ${MSVC71}" )
	dprint ( "MSVC80: ${MSVC80}" )
	dprint ( "CMAKE_COMPILER_2005: ${CMAKE_COMPILER_2005}" )
endif(WINDOWS)

# set this to true if you don't want to rebuild the object files if the rules have changed, 
# but not the actual source files or headers (e.g. if you changed the some compiler switches) 
dprint ( "CMAKE_SKIP_RULE_DEPENDENCY: ${CMAKE_SKIP_RULE_DEPENDENCY}" )

# since CMake 2.1 the install rule depends on all, i.e. everything will be built before installing. 
# If you don't like this, set this one to true.
dprint ( "CMAKE_SKIP_INSTALL_ALL_DEPENDENCY: ${CMAKE_SKIP_INSTALL_ALL_DEPENDENCY}" )

# If set, runtime paths are not added when using shared libraries. Default it is set to OFF
dprint ( "CMAKE_SKIP_RPATH: ${CMAKE_SKIP_RPATH}" )

# set this to true if you are using makefiles and want to see the full compile and link 
# commands instead of only the shortened ones 
dprint ( "CMAKE_VERBOSE_MAKEFILE: ${CMAKE_VERBOSE_MAKEFILE}" )

# this will cause CMake to not put in the rules that re-run CMake. This might be useful if 
# you want to use the generated build files on another machine. 
dprint ( "CMAKE_SUPPRESS_REGENERATION: ${CMAKE_SUPPRESS_REGENERATION}" )

# A simple way to get switches to the compiler is to use ADD_DEFINITIONS(). 
# But there are also two variables exactly for this purpose: 

# the compiler flags for compiling C sources 
dprint ( "CMAKE_C_FLAGS: ${CMAKE_C_FLAGS}" )

# the compiler flags for compiling C++ sources
dprint ( "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}" )

# Choose the type of build.  Example: SET(CMAKE_BUILD_TYPE Debug) 
dprint ( "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}" )

# if this is set to ON, then all libraries are built as shared libraries by default.
dprint ( "BUILD_SHARED_LIBS: ${BUILD_SHARED_LIBS}" )

# the compiler used for C files 
dprint ( "CMAKE_C_COMPILER: ${CMAKE_C_COMPILER}" )

# version information about the compiler
dprint ( "CMAKE_C_COMPILER_VERSION: ${CMAKE_C_COMPILER_VERSION}" )

# the compiler used for C++ files 
dprint ( "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}" )

# version information about the compiler
dprint ( "CMAKE_CXX_COMPILER_VERSION: ${CMAKE_CXX_COMPILER_VERSION}" )

# if the compiler is a variant of gcc, this should be set to 1 
dprint ( "CMAKE_COMPILER_IS_GNUCC: ${CMAKE_COMPILER_IS_GNUCC}" )

# if the compiler is a variant of g++, this should be set to 1 
dprint ( "CMAKE_COMPILER_IS_GNUCXX : ${CMAKE_COMPILER_IS_GNUCXX}" )

# the tools for creating libraries 
dprint ( "CMAKE_AR: ${CMAKE_AR}" )
dprint ( "CMAKE_RANLIB: ${CMAKE_RANLIB}" )

dprint("----- Begin CMake Var DUMP -----")

message(STATUS "********* ENDING CONFIGURATION *********")