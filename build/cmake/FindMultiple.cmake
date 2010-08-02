MACRO (FIND_MULTIPLE _NAMES _VAR_FOUND)

	## Normally find_library will exist on 1st match, for some windows libs 
	## there is one target and multiple libs

	foreach ( loop_var ${_NAMES} )
	
		find_library( ${_VAR_FOUND}_SEARCH_${loop_var} NAMES ${loop_var} )

			if (NOT ${${_VAR_FOUND}_SEARCH_${loop_var}} STREQUAL "${_VAR_FOUND}_SEARCH_${loop_var}-NOTFOUND" )

				if (${_VAR_FOUND})
					list(APPEND ${_VAR_FOUND} ${${_VAR_FOUND}_SEARCH_${loop_var}} )
				else()
					set (${_VAR_FOUND} ${${_VAR_FOUND}_SEARCH_${loop_var}} )
				endif()
			endif()

	endforeach(loop_var)

	if (${_VAR_FOUND})
		set (TEMPLST ${_NAMES})
		list(LENGTH TEMPLST NUM_SEARCH)
		list(LENGTH ${_VAR_FOUND} NUM_FOUND)
		dprint("searching[${NUM_SEARCH}]:(${_NAMES}) ... found[${NUM_FOUND}]:(${${_VAR_FOUND}})")
	else()
		if (SOFT_IS_HARD)
			message(FATAL_ERROR "Could not find libs(${_NAMES})")
		else()
			message(STATUS "Could not find libs(${_NAMES})")
		endif()
	endif()

ENDMACRO (FIND_MULTIPLE)
