if (HAVE_EXT_POSTGRESQL)

	condor_glob( dbmsdHdrs dbmsdSrcs "" )

	condor_exe( condor_dbmsd "${dbmsdHdrs};${dbmsdSrcs}" OFF "tt;${PCRE_FOUND};${CONDOR_LIBS};${OPENSSL_FOUND};${COREDUMPER_FOUND};${KRB5_FOUND};${POSTGRESQL_FOUND}")

else()

	message(STATUS "NOTE: *dbmsd* targets will not be built HAVE_EXT_POSTGRESQL=!DEFINED ")

endif()