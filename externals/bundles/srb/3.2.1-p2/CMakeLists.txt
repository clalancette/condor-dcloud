option(WITH_SRB "Compiling with support for SRB" OFF)

if ( NOT PROPER AND WITH_SRB )

	set (SRB_VER srb-3.2.1)

	ExternalProject_Add(srb
						DEPENDS openssl globus
			    		PREFIX ${EXTERNAL_BUILD_PREFIX}/${SRB_VER}
			   			 #-- Download Step ----------
						DOWNLOAD_COMMAND wget -N http://parrot.cs.wisc.edu/externals/${SRB_VER}.tar.gz ${CMD_TERM}
			    		DOWNLOAD_DIR ${EXTERNAL_DL}
			    		URL http://parrot.cs.wisc.edu/externals/${SRB_VER}.tar.gz
						#--Patch step ----------
						PATCH_COMMAND patch -i ${CMAKE_CURRENT_SOURCE_DIR}/gsi.patch -p0
						#--Configure step ----------
						CONFIGURE_COMMAND cd SRB3_2_1e &&
						./configure --prefix=${EXTERNAL_STAGE}
						--enable-gsi-auth
						--enable-globus-location=${EXTERNAL_STAGE}
						--enable-installdir=${EXTERNAL_STAGE}
						--enable-secure-comm
						--enable-openssl-location=${EXTERNAL_STAGE}
						#--Build Step ----------
						BUILD_COMMAND cd SRB3_2_1e && make
						BUILD_IN_SOURCE 1
						#--install Step ----------
						INSTALL_DIR ${EXTERNAL_STAGE}
 						INSTALL_COMMAND cd SRB3_2_1e && make install )

	message (STATUS "external configured (srb)")
	set( SRB_FOUND ${SRB_FOUND} PARENT_SCOPE )
	set( HAVE_EXT_SRB ON PARENT_SCOPE )
	append_var(CONDOR_EXTERNALS srb)

else()

	message (STATUS "external skiped (srb)")

endif()
