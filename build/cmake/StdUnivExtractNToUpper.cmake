MACRO ( STDU_EXTRACT_AND_TOUPPER _LIB _OBJ_FILE _SYMBOL_LIST  )

	# extract from the library.
	set (_OBJCOPY_COMMAND objcopy)

	foreach ( _ITEM ${_SYMBOL_LIST} )
		string( TOUPPER "${_ITEM}" NEW_UPPER_ITEM_NAME )
		set (_OBJCOPY_COMMAND ${_OBJCOPY_COMMAND} --redefine-sym ${_ITEM}=${NEW_UPPER_ITEM_NAME})
	endforeach(_ITEM)

	# with one command ar extract && objcopy redefine symbols ToUpper.
	add_custom_command( OUTPUT ${_OBJ_FILE}
				COMMAND ar
				ARGS -x ${_LIB} ${_OBJ_FILE} && ${_OBJCOPY_COMMAND} ${_OBJ_FILE} )

	dprint("Symbol extraction = ar -x ${_LIB} ${_OBJ_FILE} && ${_OBJCOPY_COMMAND} ${_OBJ_FILE}")
	append_var(STDU_OBJS ${_OBJ_FILE})

ENDMACRO ( STDU_EXTRACT_AND_TOUPPER )
