MACRO ( DUMP_LOCAL_FLAGS _DUMP_VAR )

	get_directory_property(CDEFS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} COMPILE_DEFINITIONS)
	get_directory_property(IDEFS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} INCLUDE_DIRECTORIES)

	#loop through -D Vars
	foreach ( _INDEX ${CDEFS} )

		if(${_DUMP_VAR})
			set (${_DUMP_VAR} "${${_DUMP_VAR}};-D${_INDEX}")
		else(${_DUMP_VAR})
			set (${_DUMP_VAR} -D${_INDEX})
		endif(${_DUMP_VAR})

	endforeach (_INDEX)

	# loop through -I Vars
	foreach ( _INDEX ${IDEFS} )

		if(${_DUMP_VAR})
			set (${_DUMP_VAR} "${${_DUMP_VAR}};-I${_INDEX}")
		else(${_DUMP_VAR})
			set (${_DUMP_VAR} -I${_INDEX})
		endif(${_DUMP_VAR})

	endforeach (_INDEX)

ENDMACRO ( DUMP_LOCAL_FLAGS )