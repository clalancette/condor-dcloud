if (HAVE_EXT_GLOBUS)

	 install(FILES gt42-gahp.jar
			 DESTINATION ${C_LIB} )

else()

	message(STATUS "NOTE: *gt42* targets will not be built HAVE_EXT_GLOBUS=${HAVE_EXT_GLOBUS}")

endif (HAVE_EXT_GLOBUS)