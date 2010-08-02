
file( GLOB DCRmvElements soap_daemon_core* condor_softkill*)

condor_glob( DCHeaderFiles DCSourceFiles "${DCRmvElements};soap_core.cpp;mimetypes.cpp" )

#On windows we add the cpp file into the build 
if (GSOAP_SRC)
	set_source_files_properties(${GSOAP_SRC} PROPERTIES GENERATED TRUE)
	list(APPEND DCSourceFiles ${GSOAP_SRC} )
endif()

#create the library
condor_static_lib(daemon_core "${DCHeaderFiles};${DCSourceFiles}")

if (WINDOWS)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
	condor_exe( condor_softkill "condor_softkill.WINDOWS.cpp;condor_softkill.h" ${C_SBIN} "${CONDOR_TOOL_LIBS};psapi")
endif(WINDOWS)
