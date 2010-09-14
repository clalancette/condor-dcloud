##########################################################################
# adds all subtree elements which have a CMakeLists.txt file exclude
# classads b/c we added earlier to elim externals dependency - all externals reference
# one version of classads which is ${CONDOR_SOURCE_DIR}/src/classad

# the 2nd arg is prio order for symbols ripping to prevent .o copy-mindging within the std:u
add_subdirectories("${CONDOR_SOURCE_DIR}/src" "${CONDOR_SOURCE_DIR}/src/condor_utils;${CONDOR_SOURCE_DIR}/src/condor_ckpt;${CONDOR_SOURCE_DIR}/src/condor_io;${CONDOR_SOURCE_DIR}/src/condor_io.std;${CONDOR_SOURCE_DIR}/src/condor_syscall_lib" "${CONDOR_SOURCE_DIR}/src/classad")

if (BUILD_TESTS)
	add_custom_target(tests)
	add_dependencies(tests ${CONDOR_TESTS})
	dprint("CONDOR_TESTS=${CONDOR_TESTS}")
endif(BUILD_TESTS)

