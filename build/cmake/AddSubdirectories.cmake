macro(add_subdirectories rootdir prio exclude)
 file(GLOB localsub "${rootdir}/*" )

  foreach(_item ${prio})
	message(STATUS "adding directory (${_item})")
	add_subdirectory( ${_item} )
	list(REMOVE_ITEM localsub ${_item})
  endforeach(_item)

  foreach(dir ${localsub})
    if(IS_DIRECTORY ${dir})
        
	foreach (exclude_var ${exclude})
		if ( ${dir} STREQUAL ${exclude_var})
			message(STATUS "excluding directory (${dir})")
        		set( ${dir}_exclude ON )
		endif()
	endforeach(exclude_var)

	if ( EXISTS ${dir}/CMakeLists.txt AND NOT ${dir}_exclude )
		message(STATUS "adding directory (${dir})")
        	add_subdirectory( ${dir} )
	endif()

    endif()
  endforeach(dir)
endmacro()
