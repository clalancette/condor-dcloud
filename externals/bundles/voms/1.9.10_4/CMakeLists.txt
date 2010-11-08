
option( WITH_VOMS "Compiling with support for VOMS" ON )

if ( LINUX AND WITH_VOMS )

	if (NOT PROPER)

		set (VOMS_VER voms-1.9.10_4)

		ExternalProject_Add(voms
							DEPENDS expat openssl globus zlib
							PREFIX ${EXTERNAL_BUILD_PREFIX}/${VOMS_VER}
							#-- Download Step ----------
							DOWNLOAD_COMMAND wget -N http://parrot.cs.wisc.edu/externals/${VOMS_VER}.tar.gz ${CMD_TERM}
							DOWNLOAD_DIR ${EXTERNAL_DL}
							URL http://parrot.cs.wisc.edu/externals/${VOMS_VER}.tar.gz
							#--Patch step ----------
							PATCH_COMMAND patch -p0 < ${CMAKE_CURRENT_SOURCE_DIR}/configure.patch &&
								patch -p0 < ${CMAKE_CURRENT_SOURCE_DIR}/makefile.patch &&
								patch -p0 < ${CMAKE_CURRENT_SOURCE_DIR}/private.patch &&
								${CMAKE_CURRENT_SOURCE_DIR}/voms_file_patch.sh
							#--Configure step ----------
							CONFIGURE_COMMAND cd org.glite.security.voms &&
								./configure
								--prefix=${EXTERNAL_STAGE}
								--with-api-only
								--with-openssl-prefix=${EXTERNAL_STAGE}
								--with-expat-prefix=${EXTERNAL_STAGE}
								--with-globus-prefix=${EXTERNAL_STAGE}
								--with-globus-flavor=${GLOBUS_FLAVOR} --disable-java --disable-docs --disable-glite
							#--Build Step ----------
							BUILD_COMMAND cd org.glite.security.voms && make
							BUILD_IN_SOURCE 1
							#--install Step ----------
							INSTALL_DIR ${EXTERNAL_STAGE}
							INSTALL_COMMAND cd org.glite.security.voms && make install
				)

		set( VOMS_FOUND "libvomsapi_${GLOBUS_FLAVOR}.a" )

	else(NOT PROPER)

		find_multiple( "vomsapi" VOMS_FOUND)

	endif(NOT PROPER)

	if (VOMS_FOUND)
		message ( STATUS "external configured (VOMS_FOUND=${VOMS_FOUND})" )
		set( HAVE_EXT_VOMS ON PARENT_SCOPE )
		set( VOMS_FOUND ${VOMS_FOUND} PARENT_SCOPE )
		append_var(CONDOR_EXTERNALS voms)
		append_var(EXTERNAL_INCLUDES ${EXTERNAL_STAGE}/include/glite/security)
	endif()

endif()

if(NOT VOMS_FOUND)
	message ( STATUS "external skiped (voms)" )
endif()
