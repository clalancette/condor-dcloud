 ###############################################################
 # 
 # Copyright 2011 Red Hat, Inc. 
 # 
 # Licensed under the Apache License, Version 2.0 (the "License"); you 
 # may not use this file except in compliance with the License.  You may 
 # obtain a copy of the License at 
 # 
 #    http://www.apache.org/licenses/LICENSE-2.0 
 # 
 # Unless required by applicable law or agreed to in writing, software 
 # distributed under the License is distributed on an "AS IS" BASIS, 
 # WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 # See the License for the specific language governing permissions and 
 # limitations under the License. 
 # 
 ############################################################### 

# The source tarball for this external was downloaded from this URL:
#   http://jra1mw.cvs.cern.ch/cgi-bin/jra1mw.cgi/org.glite.ce.blahp.tar.gz?view=tar&pathrev=glite-ce-blahp_R_1_16_1_0

if ( LINUX )
	option(WITH_BLAHP "Compiling with support for BLAHP" ON)
endif ( LINUX )

if ( WITH_BLAHP )

	if ( NOT PROPER )
		message(STATUS "TODO:TSTCLAIR BLAHP eval if linking correct popt w/o mundging Makefile.am")

		set(POPT_VER popt-1.14)
		set(BLAHP_VER blahp-1.16.1)

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

		#dprint("we ingore the return param from bootstrap.. there are errors!")

		ExternalProject_Add(blahp
						DEPENDS openssl globus popt
					    PREFIX ${EXTERNAL_BUILD_PREFIX}/${BLAHP_VER}
					    #-- Download Step ----------
						DOWNLOAD_COMMAND wget -N http://parrot.cs.wisc.edu/externals/${BLAHP_VER}.tar.gz ${CMD_TERM}
					    DOWNLOAD_DIR ${EXTERNAL_DL}
						URL http://parrot.cs.wisc.edu/externals/${BLAHP_VER}.tar.gz
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

		### set the install
		install ( DIRECTORY ${EXTERNAL_STAGE}/opt/glite DESTINATION ${C_LIBEXEC} USE_SOURCE_PERMISSIONS )

		add_dependencies(blahp classads)	

		set(BLAHP_FOUND ON)

		append_var (CONDOR_EXTERNALS blahp)

	else ( NOT PROPER )

		find_program( BLAHP_FOUND BLClient HINTS /usr/bin )
		dprint("Scanning for BLClient...(${BLAHP_FOUND})")

	endif ( NOT PROPER )


endif( WITH_BLAHP )

if ( BLAHP_FOUND )

	message( STATUS "external configured (blahp) BLAHP_FOUND=${BLAHP_FOUND}" )
	set( BLAHP_FOUND ${BLAHP_FOUND} PARENT_SCOPE )
	set( HAVE_EXT_BLAHP ON PARENT_SCOPE )
else()
	message ( STATUS "external skipped (blahp)" )
endif ( BLAHP_FOUND )
	
