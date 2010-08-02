if (HAVE_EXT_GLOBUS)

	 install(FILES gt4-gahp.jar
			 DESTINATION ${C_LIB} )

else()

	message(STATUS "NOTE: *gt4* targets will not be built HAVE_EXT_GLOBUS=${HAVE_EXT_GLOBUS}")

endif (HAVE_EXT_GLOBUS)