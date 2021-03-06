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

# currently hard dependency
#option(WITH_PCRE "Compiling with support for PCRE" ON)
#if (WITH_PCRE)

	if ( NOT PROPER )

		set (PCRE_VER pcre-7.6)
		if (WINDOWS)
			set (PCRE_PATCH cp ${CMAKE_CURRENT_SOURCE_DIR}/Makefile.win32 . \r\n
							cp ${CMAKE_CURRENT_SOURCE_DIR}/pcre.h.win32 pcre.h \r\n
							cp ${CMAKE_CURRENT_SOURCE_DIR}/libpcre.def.win32 libpcre.def )
			set (PCRE_CONFIGURE cp ${CMAKE_CURRENT_SOURCE_DIR}/config.h.win32 config.h)
			set (PCRE_MAKE gmake -f Makefile.win32 )
			set (PCRE_INSTALL cp libpcre.lib ${EXTERNAL_STAGE}/lib \r\n
							  cp libpcre.dll ${EXTERNAL_STAGE}/lib \r\n
							  cp pcre.h ${EXTERNAL_STAGE}/include )
		else()
			set (PCRE_PATCH  patch --binary -p0 < ${CMAKE_CURRENT_SOURCE_DIR}/longlong.patch &&
							 patch --binary -p0 < ${CMAKE_CURRENT_SOURCE_DIR}/unittests.patch )
			set (PCRE_CONFIGURE ./configure --prefix=${EXTERNAL_STAGE} --disable-cpp )
			set (PCRE_MAKE make && make test )
			set (PCRE_INSTALL make install )
		endif()

		ExternalProject_Add(pcre
				    PREFIX ${EXTERNAL_BUILD_PREFIX}/${PCRE_VER}
				    #-- Download Step ----------
				    DOWNLOAD_COMMAND wget -N http://parrot.cs.wisc.edu/externals/${PCRE_VER}.tar.gz ${CMD_TERM}
				    DOWNLOAD_DIR ${EXTERNAL_DL}
				    URL http://parrot.cs.wisc.edu/externals/${PCRE_VER}.tar.gz
					#--Patch step ----------
					PATCH_COMMAND cd ${PCRE_VER} ${CMD_TERM} ${PCRE_PATCH}
				    #--Configure step ----------
				    CONFIGURE_COMMAND cd ${PCRE_VER} ${CMD_TERM} ${PCRE_CONFIGURE}
				    #--Build Step ----------
				    BUILD_COMMAND cd ${PCRE_VER} ${CMD_TERM} ${PCRE_MAKE}
				    BUILD_IN_SOURCE 1
				    #--install Step ----------
				    INSTALL_DIR ${EXTERNAL_STAGE}
				    INSTALL_COMMAND cd ${PCRE_VER} ${CMD_TERM} ${PCRE_INSTALL} )

		# Set the target dependencies which the rest of condor depends on.
		if (WINDOWS)
			set(PCRE_FOUND "${EXTERNAL_STAGE}/lib/libpcre.lib")

			#this may become the common case.
			install ( FILES ${EXTERNAL_STAGE}/lib/libpcre.dll DESTINATION ${C_LIB} )
			
			set_property( TARGET pcre PROPERTY FOLDER "externals" )
			
		else()
			set(PCRE_FOUND "${EXTERNAL_STAGE}/lib/libpcre.a")
			if ( AIX )
				set (PCRE_FOUND "-Wl,-bstatic;${PCRE_FOUND};-Wl,-bdynamic")
			endif()
		endif()

		append_var(CONDOR_EXTERNALS pcre)

	else( NOT PROPER )

		find_multiple( "pcre;libpcre" PCRE_FOUND )

	endif( NOT PROPER )

	if (PCRE_FOUND)
		message (STATUS "external configured (PCRE_FOUND=${PCRE_FOUND})")
		set( PCRE_FOUND ${PCRE_FOUND} PARENT_SCOPE )
		set( HAVE_EXT_PCRE ON PARENT_SCOPE )
		set( HAVE_PCRE_H ON PARENT_SCOPE )
	else()
	  message (FATAL_ERROR "pcre not found and is required to build")
	endif(PCRE_FOUND)

#else(WITH_PCRE)
#	message (STATUS "external skipped (pcre)")
#endif(WITH_PCRE)
