option(WITH_UNICOREGAHP "Compiling with support for UNICOREGAHP" ON)

if ( WITH_UNICOREGAHP AND NOT PROPER )

	set (UCGAHP_VER unicoregahp-1.2.0)
	ExternalProject_Add(unicoregahp
			    PREFIX ${EXTERNAL_BUILD_PREFIX}/${UCGAHP_VER}
			    #-- Download Step ----------
				DOWNLOAD_COMMAND wget -N http://parrot.cs.wisc.edu/externals/${UCGAHP_VER}.tar.gz ${CMD_TERM}
			    DOWNLOAD_DIR ${EXTERNAL_DL}
			    URL http://parrot.cs.wisc.edu/externals/${UCGAHP_VER}.tar.gz
				#--Configure step ----------
				CONFIGURE_COMMAND echo "nothing to do"
				#--Build Step ----------
				BUILD_COMMAND echo "nothing to do"
				BUILD_IN_SOURCE 1
				#--install Step ----------
			    INSTALL_DIR ${EXTERNAL_STAGE}/usr/share/java
			    INSTALL_COMMAND cp ugahp.jar ${EXTERNAL_STAGE}/usr/share/java )

	message (STATUS "external configured (unicoregahp)")
	set( HAVE_EXT_UNICOREGAHP ON PARENT_SCOPE )
	append_var(CONDOR_EXTERNALS unicoregahp)
	install(FILES ${EXTERNAL_STAGE}/usr/share/java/ugahp.jar DESTINATION ${C_LIB})

else()
	message (STATUS "external skiped (unicoregahp)")
endif()