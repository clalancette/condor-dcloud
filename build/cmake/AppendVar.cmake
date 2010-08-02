MACRO (APPEND_VAR _VAR _VAL)

	if(${_VAR})
		set (${_VAR} "${${_VAR}};${_VAL}")
	else(${_VAR})
		set (${_VAR} ${_VAL})
	endif(${_VAR})

	set (${_VAR} ${${_VAR}} PARENT_SCOPE )

ENDMACRO(APPEND_VAR)