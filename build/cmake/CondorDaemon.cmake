MACRO (CONDOR_DAEMON _CNDR_TARGET _REMOVE_ELEMENTS _LINK_LIBS _INSTALL_LOC _GEN_GSOAP )

	set(${_CNDR_TARGET}SOAP ${_GEN_GSOAP})
	set(${_CNDR_TARGET}_REMOVE_ELEMENTS ${_REMOVE_ELEMENTS})

    if (NOT ${_CNDR_TARGET}SOAP OR NOT HAVE_EXT_GSOAP )
		file ( GLOB ${_CNDR_TARGET}_REMOVE_ELEMENTS *soap* ${_REMOVE_ELEMENTS} )
    endif()

	condor_glob( ${_CNDR_TARGET}HDRS ${_CNDR_TARGET}SRCS "${${_CNDR_TARGET}_REMOVE_ELEMENTS}" )

    if ( ${_CNDR_TARGET}SOAP AND HAVE_EXT_GSOAP )
		gsoap_gen( ${_CNDR_TARGET} ${_CNDR_TARGET}HDRS ${_CNDR_TARGET}SRCS )
		list(APPEND ${_CNDR_TARGET}SRCS ${DAEMON_CORE}/soap_core.cpp ${DAEMON_CORE}/mimetypes.cpp)
		list(APPEND ${_CNDR_TARGET}HDRS ${DAEMON_CORE}/soap_core.h ${DAEMON_CORE}/mimetypes.h)
	endif()

	#Add the executable target.
	condor_exe( condor_${_CNDR_TARGET} "${${_CNDR_TARGET}HDRS};${${_CNDR_TARGET}SRCS}" ${_INSTALL_LOC} "${_LINK_LIBS}")

	# update the dependencies based on options
	if ( ${_CNDR_TARGET}SOAP AND HAVE_EXT_GSOAP)
		add_dependencies(condor_${_CNDR_TARGET} gen_${_CNDR_TARGET}_soapfiles)
	endif()
	
ENDMACRO (CONDOR_DAEMON)