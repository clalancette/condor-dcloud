option( WITH_VOMS "Compiling with support for VOMS" OFF )

if ( NOT PROPER AND WITH_VOMS )

	set (VOMS_VER voms-1.8.8_2)

	message ( STATUS "TODO: Crazy for loop of OMG this is terrible" )

	ExternalProject_Add(voms
						DEPENDS expat openssl globus
					    PREFIX ${EXTERNAL_BUILD_PREFIX}/${VOMS_VER}
					    #-- Download Step ----------
						DOWNLOAD_COMMAND wget -N http://parrot.cs.wisc.edu/externals/${VOMS_VER}.tar.gz ${CMD_TERM}
					    DOWNLOAD_DIR ${EXTERNAL_DL}
					    URL http://parrot.cs.wisc.edu/externals/${VOMS_VER}.tar.gz
						#--Patch step ----------
						PATCH_COMMAND patch -p0 < ${CMAKE_CURRENT_SOURCE_DIR}/configure.patch &&
						 patch -p0 < ${CMAKE_CURRENT_SOURCE_DIR}/includes.patch &&
						 patch -p0 < ${CMAKE_CURRENT_SOURCE_DIR}/makefile.patch
				    	#--Configure step ----------
					    CONFIGURE_COMMAND cd org.glite.security.voms && ./configure --prefix=${EXTERNAL_STAGE} --with-api-only
						 --with-openssl-prefix=${EXTERNAL_STAGE}
						 --with-expat-prefix=${EXTERNAL_STAGE}
						 --with-globus-prefix=${EXTERNAL_STAGE}
						 --disable-java --disable-docs --disable-glite
					    #--Build Step ----------
					    BUILD_COMMAND cd org.glite.security.voms && make
					    BUILD_IN_SOURCE 1
					    #--install Step ----------
					    INSTALL_DIR ${EXTERNAL_STAGE}
					    INSTALL_COMMAND cd org.glite.security.voms && make install )

	set( VOMS_FOUND "${EXTERNAL_STAGE}/lib/libvomsapi_${GLOBUS_FLAVOR}.a" )
	message ( STATUS "external configured (VOMS_FOUND=${VOMS_FOUND})" )
	set( HAVE_EXT_VOMS ON PARENT_SCOPE )
	set( VOMS_FOUND ${VOMS_FOUND} PARENT_SCOPE )
	append_var(CONDOR_EXTERNALS voms)

else()
	message ( STATUS "external skiped (voms)" )
endif()