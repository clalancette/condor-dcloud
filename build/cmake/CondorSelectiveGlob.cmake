MACRO (CONDOR_SELECTIVE_GLOB _LIST _SRCS)
	string(TOUPPER "${CMAKE_SYSTEM_NAME}" TEMP_OS)

	file(GLOB WinSrcs "*WINDOWS*" "*windows*")
	file(GLOB LinuxSrcs "*LINUX*" "*linux*")
	file(GLOB UnixSrcs "*UNIX*" "*unix*")
	file(GLOB RmvSrcs "*.dead*" "*NON_POSIX*" "*.o" "*.lo" "*.a" "*.la" "*.dll" "*.lib")

	foreach ( _ITEM ${_LIST} )

		# clear the temp var
		set(_TEMP OFF)
		file ( GLOB _TEMP ${_ITEM} )

		if (_TEMP)
			# brute force but it works, and cmake is a one time pad
			if(RmvSrcs)
				list(REMOVE_ITEM _TEMP ${RmvSrcs})
			endif()

			if(${TEMP_OS} STREQUAL "LINUX")
				if (WinSrcs)
					list(REMOVE_ITEM _TEMP ${WinSrcs})
				endif()
				# linux vers *will build *.unix.* as was done before
			elseif (${TEMP_OS} STREQUAL "WINDOWS")
				if (LinuxSrcs)
					list(REMOVE_ITEM _TEMP ${LinuxSrcs})
				endif()
				if (UnixSrcs)
					list(REMOVE_ITEM _TEMP ${UnixSrcs})
				endif()
			else()
				if (WinSrcs)
					list(REMOVE_ITEM _TEMP ${WinSrcs})
				endif()
				if (LinuxSrcs)
					list(REMOVE_ITEM _TEMP ${LinuxSrcs})
				endif()
			endif()

			# now tack on the items.
			list(APPEND ${_SRCS} ${_TEMP} )

		endif(_TEMP)
	endforeach(_ITEM)

	list (REMOVE_DUPLICATES ${_SRCS})

ENDMACRO (CONDOR_SELECTIVE_GLOB)