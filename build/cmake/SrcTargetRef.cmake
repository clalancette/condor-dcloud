MACRO ( SRC_TARGET_REF _TARGET _REFS_EXPR _SRCREFS )

	# 1st obtain the srcs
	get_target_property ( _LSRCS ${_TARGET} SOURCES )

	#loop through srouces and check matches
	foreach ( _TARGET_SRC_FILE ${_LSRCS} )

		#for each src file check all refs.
		foreach ( _REF ${_REFS_EXPR} )

			# if matches 
			if(${_TARGET_SRC_FILE} MATCHES ${_REF})

				if(${_SRCREFS})
					set (${_SRCREFS} "${${_SRCREFS}};${_TARGET_SRC_FILE}")
				else(${_SRCREFS})
					set (${_SRCREFS} ${_TARGET_SRC_FILE})
				endif(${_SRCREFS})

			endif()

		endforeach( _REF )
	endforeach( _TARGET_SRC_FILE )

ENDMACRO ( SRC_TARGET_REF )