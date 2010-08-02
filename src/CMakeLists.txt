##########################################################################
# adds all subtree elements which have a CMakeLists.txt file exclude
# classads b/c we added earlier to elim externals dependency - all externals reference
# one version of classads which is ${CONDOR_SOURCE_DIR}/src/classad
add_subdirectories("${CONDOR_SOURCE_DIR}/src" "${CONDOR_SOURCE_DIR}/src/classad")

if (BUILD_TESTS)
	add_custom_target(tests)
	add_dependencies(tests ${CONDOR_TESTS})
	dprint("CONDOR_TESTS=${CONDOR_TESTS}")
endif(BUILD_TESTS)

