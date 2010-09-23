option(WITH_BLAHP "Compiling with support for BLAHP" ON)

if ( NOT PROPER AND WITH_BLAHP )

	message(STATUS "TODO:TSTCLAIR BLAHP eval if linking correct popt w/o mundging Makefile.am")
	
	set(POPT_VER popt-1.14)
	set(BLAHP_VER blahp-1.16.0)

	ExternalProject_Add(popt
						PREFIX ${EXTERNAL_BUILD_PREFIX}/${POPT_VER}
						#-- Download Step ----------
						DOWNLOAD_COMMAND wget -N http://parrot.cs.wisc.edu/externals/${POPT_VER}.tar.gz ${CMD_TERM}
						DOWNLOAD_DIR ${EXTERNAL_DL}
						URL http://parrot.cs.wisc.edu/externals/${POPT_VER}.tar.gz
						#--Configure step ----------
			    		CONFIGURE_COMMAND cd ${POPT_VER} && ./configure --prefix=${EXTERNAL_STAGE}
						#--Build Step ----------
						BUILD_COMMAND cd ${POPT_VER} && make
						BUILD_IN_SOURCE 1
						#--install Step ----------
						INSTALL_DIR ${EXTERNAL_STAGE}
						INSTALL_COMMAND cd ${POPT_VER} && make install )

	dprint("we ingore the return param from bootstrap.. there are errors!")

	ExternalProject_Add(blahp
						DEPENDS classads openssl globus popt
					    PREFIX ${EXTERNAL_BUILD_PREFIX}/${BLAHP_VER}
					    #-- Download Step ----------
						DOWNLOAD_COMMAND wget -N http://pages.cs.wisc.edu/~jfrey/externals/${BLAHP_VER}.tar.gz ${CMD_TERM}
					    DOWNLOAD_DIR ${EXTERNAL_DL}
						URL http://pages.cs.wisc.edu/~jfrey/externals/${BLAHP_VER}.tar.gz
						#--Patch step ----------
						PATCH_COMMAND patch -i ${CMAKE_CURRENT_SOURCE_DIR}/classad.patch -p0 &&
							patch -i ${CMAKE_CURRENT_SOURCE_DIR}/mtrace.patch -p0 &&
							patch -i ${CMAKE_CURRENT_SOURCE_DIR}/registry.patch -p0 && 
							patch -i ${CMAKE_CURRENT_SOURCE_DIR}/dl.patch -p0 &&
							cp ${CMAKE_CURRENT_SOURCE_DIR}/classads.m4 org.glite.ce.blahp/project &&
							cp ${CMAKE_CURRENT_SOURCE_DIR}/globus.m4 org.glite.ce.blahp/project &&
							cp ${CMAKE_CURRENT_SOURCE_DIR}/glite.m4 org.glite.ce.blahp/project.&&
							patch -i ${CMAKE_CURRENT_SOURCE_DIR}/name-change.patch -p0 &&
							mv org.glite.ce.blahp/config/blah.config.template org.glite.ce.blahp/config/batch_gahp.config.template
						#--Configure step ----------
						CONFIGURE_COMMAND cd org.glite.ce.blahp &&
							./bootstrap || ./configure --with-classads-prefix=${EXTERNAL_STAGE}
							--with-openssl-prefix=${EXTERNAL_STAGE}
							--with-globus-prefix=${EXTERNAL_STAGE}
							--with-globus-nothr-flavor=gcc${BIT_MODE}dbg
							--prefix=${EXTERNAL_STAGE}/opt/glite
						#--Build Step ----------
						BUILD_COMMAND cd org.glite.ce.blahp && make
						BUILD_IN_SOURCE 1
						#--install Step ----------
						INSTALL_DIR ${EXTERNAL_STAGE}
						INSTALL_COMMAND cd org.glite.ce.blahp && make install )

	message( STATUS "external configured (blahp)" )
	set( HAVE_EXT_BLAHP ON PARENT_SCOPE )
	append_var (CONDOR_EXTERNALS blahp)

	### set the install
	install ( DIRECTORY ${EXTERNAL_STAGE}/opt/glite DESTINATION ${C_LIB} )

else( NOT PROPER AND WITH_BLAHP )

	message ( STATUS "external skiped (blahp)" )

endif( NOT PROPER AND WITH_BLAHP )