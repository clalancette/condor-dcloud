# Generate currente date for using in each type of package
# May need to change to add_custom_target instead so that date is captured at build time

MACRO (PACKAGEDATE _TYPE _VAR )	

	if ( ${_TYPE} STREQUAL  "RPM" )
		execute_process(
			COMMAND date "+%a %b %d %Y"			
			OUTPUT_VARIABLE ${_VAR}
			OUTPUT_STRIP_TRAILING_WHITESPACE
			)
	elseif ( ${_TYPE} STREQUAL  "DEB" )
		execute_process(
			COMMAND date -R			
			OUTPUT_VARIABLE ${_VAR}
			OUTPUT_STRIP_TRAILING_WHITESPACE
			)
	endif()

	#message (STATUS ${TYPE} ${${_VAR}})

ENDMACRO (PACKAGEDATE)