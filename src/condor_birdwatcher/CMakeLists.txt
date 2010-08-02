if (WINDOWS AND NOT WIN_EXEC_NODE_ONLY)

	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
	
	file( GLOB BWSrcs *.cpp *.h  res/*)
	
	condor_exe( condor_birdwatcher "${BWSrcs}" ${C_SBIN} "")

else()

	message(STATUS "NOTE: *birdwatcher* targets will not be built WINDOWS=${WINDOWS}")

endif()