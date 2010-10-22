
MACRO (CONDOR_SET_LINK_LIBS _CNDR_TARGET _LINK_LIBS)

set(${_CNDR_TARGET}LinkLibs ${_LINK_LIBS})

if (${_CNDR_TARGET}LinkLibs)
        list(REMOVE_DUPLICATES ${_CNDR_TARGET}LinkLibs)

	 if ( NOT WINDOWS )
		if (DARWIN)
			target_link_libraries( ${_CNDR_TARGET} ${${_CNDR_TARGET}LinkLibs} ${${_CNDR_TARGET}LinkLibs}  )
		else()
	 		target_link_libraries( ${_CNDR_TARGET} -Wl,--start-group ${${_CNDR_TARGET}LinkLibs} -Wl,--end-group )
		endif()
	 else()
	 	target_link_libraries( ${_CNDR_TARGET} ${${_CNDR_TARGET}LinkLibs};${CONDOR_WIN_LIBS} )
	 endif()
endif()

ENDMACRO (CONDOR_SET_LINK_LIBS)
