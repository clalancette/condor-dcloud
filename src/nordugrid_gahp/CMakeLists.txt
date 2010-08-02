if (HAVE_EXT_GLOBUS AND HAVE_LDAP_H)

	add_definitions(-DLDAP_DEPRECATED)

	condor_glob( HeaderFiles SourceFiles "" )

	condor_exe( nordugrid_gahp
				"${HeaderFiles};${SourceFiles}"
				${C_SBIN}
				"${CONDOR_TOOL_LIBS};${GLOBUS_GRID_UNIVERSE_COMMON};${GLOBUS_GRID_UNIVERSE_NORUID};ldap" )

else()

	message( STATUS "NOTE: *nordugrid* targets will not be built HAVE_EXT_GLOBUS=${HAVE_EXT_GLOBUS} HAVE_LDAP_H=${HAVE_LDAP_H}" )

endif ()
