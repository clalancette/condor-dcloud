MACRO ( STDU_EXTRACT_AND_TOUPPER _LIB _OBJ_FILE _SYMBOL_LIST  )

	#step 1 ar extract object module from archive
	exec_program ( ar ARGS -x ${_LIB} ${_OBJ_FILE} )

	foreach ( _ITEM ${_SYMBOL_LIST} )

		# uppercase the string.
		string( TOUPPER "${_ITEM}" NEW_UPPER_ITEM_NAME )

		exec_program ( objcopy ARGS --redefine-sym ${_ITEM}=${NEW_UPPER_ITEM_NAME} ${_OBJ_FILE} )

	endforeach(_ITEM)

	# now append to a global var to which can be used elsewhere.
	append_var(STDU_OBJS _OBJ_FILE)

ENDMACRO ( STDU_EXTRACT_AND_TOUPPER )