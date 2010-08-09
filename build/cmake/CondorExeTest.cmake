MACRO (CONDOR_EXE_TEST _CNDR_TARGET _SRCS _LINK_LIBS )

	if (BUILD_TESTS)

		set ( LOCAL_${_CNDR_TARGET} ${_CNDR_TARGET} )

		if ( WINDOWS )
			string (REPLACE ".exe" "" ${LOCAL_${_CNDR_TARGET}} ${LOCAL_${_CNDR_TARGET}})
		endif( WINDOWS )

		add_executable( ${LOCAL_${_CNDR_TARGET}} EXCLUDE_FROM_ALL ${_SRCS})

		set_target_properties( ${LOCAL_${_CNDR_TARGET}} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_TARGET_DIR} )

		condor_set_link_libs( ${LOCAL_${_CNDR_TARGET}} "${_LINK_LIBS}" )

		# will tack onto a global var which will be *all test targets.
		if ( CONDOR_TESTS )
			set ( CONDOR_TESTS "${CONDOR_TESTS};${_CNDR_TARGET}" )
		else( CONDOR_TESTS )
			set ( CONDOR_TESTS ${_CNDR_TARGET} )
		endif( CONDOR_TESTS )

		set ( CONDOR_TESTS ${CONDOR_TESTS} PARENT_SCOPE )

	endif(BUILD_TESTS)

ENDMACRO(CONDOR_EXE_TEST)