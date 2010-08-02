
if (HAVE_EXT_GLOBUS)

	condor_exe( gahp_server "gahp_server.cpp;my_ez.cpp" ${C_SBIN}
	"${CONDOR_LIBS};${GLOBUS_GRID_UNIVERSE_COMMON};${GLOBUS_GRID_UNIVERSE_GT2};${OPENSSL_FOUND}" )

else()

	message(STATUS "NOTE: *gt2* targets will not be built HAVE_EXT_GLOBUS=${HAVE_EXT_GLOBUS}")

endif (HAVE_EXT_GLOBUS)
