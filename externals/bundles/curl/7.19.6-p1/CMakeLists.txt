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



option(WITH_CURL "Compiling with support for CURL" ON)

if (WITH_CURL)

	if ( NOT PROPER )

		if (WINDOWS)
			set (CURL_PATCH pushd "lib" ${CMD_TERM}
						patch --binary -p0 -i ${CMAKE_CURRENT_SOURCE_DIR}/Makefile.vc9.patch ${CMD_TERM}
						popd)
			set (CURL_CONFIGURE echo "Nothing to configure")
			set (CURL_MAKE pushd "lib" ${CMD_TERM}
						nmake -f Makefile.vc9 CFG=release-ssl WINDOWS_SDK_PATH="${WIN_SDK_PATH}" OPENSSL_PATH="${EXTERNAL_STAGE}" ${CMD_TERM}
						popd ${CMD_TERM})
			set (CURL_DEPENDS openssl)
			set (CURL_INSTALL cp -f lib/lib*.lib ${EXTERNAL_STAGE}/lib && cp -rf include/curl ${EXTERNAL_STAGE}/include )
		else()
			set (CURL_PATCH echo "Nothing to patch on Unix")
			set (CURL_CONFIGURE PKG_CONFIG_PATH=${EXTERNAL_STAGE} LIBS=-ldl ./configure --prefix=${EXTERNAL_STAGE} --with-ssl=${EXTERNAL_STAGE} --disable-shared --disable-ipv6 )
			set (CURL_MAKE make)
			set (CURL_INSTALL make install)
			set (CURL_DEPENDS openssl zlib)
		endif()

		set (CURL_VER curl-7.19.6)

		ExternalProject_Add(curl
							DEPENDS ${CURL_DEPENDS}
							PREFIX ${EXTERNAL_BUILD_PREFIX}/${CURL_VER}
							#-- Download Step ----------
							DOWNLOAD_COMMAND wget -N http://parrot.cs.wisc.edu/externals/${CURL_VER}.tar.gz ${CMD_TERM}
							DOWNLOAD_DIR ${EXTERNAL_DL}
							URL http://parrot.cs.wisc.edu/externals/${CURL_VER}.tar.gz
							#--Patch Step ----------
							PATCH_COMMAND cd ${CURL_VER} ${CMD_TERM} ${CURL_PATCH}
							#--Configure step ----------
							CONFIGURE_COMMAND cd ${CURL_VER} && ${CURL_CONFIGURE}
							#--Build Step ----------
							BUILD_COMMAND cd ${CURL_VER} && ${CURL_MAKE}
							BUILD_IN_SOURCE 1
							#--install Step ----------
							INSTALL_DIR ${EXTERNAL_STAGE}
							INSTALL_COMMAND cd ${CURL_VER} && ${CURL_INSTALL} )

		# Set the target dependencies which the rest of condor depends on.
		if (WINDOWS)
			set(CURL_FOUND "${EXTERNAL_STAGE}/lib/libcurl.lib")
			set_property( TARGET curl PROPERTY FOLDER "externals" )
		else()
			# libcurl is dependent on these libraries on some
			# platforms.
			find_multiple( "rt;idn" CURL_FOUND )
			set(CURL_FOUND "${EXTERNAL_STAGE}/lib/libcurl.a;${CURL_FOUND}")
		endif()

		append_var (CONDOR_EXTERNALS curl)


	else( NOT PROPER )

		#find the following libraries.
		find_multiple( "curl" CURL_FOUND)

	endif( NOT PROPER )	

	if (CURL_FOUND)
		set( CURL_FOUND ${CURL_FOUND} PARENT_SCOPE )
		set( HAVE_EXT_CURL ON PARENT_SCOPE )
		message (STATUS "external configured (CURL_FOUND=${CURL_FOUND})")
	endif(CURL_FOUND)

else(WITH_CURL)

	message (STATUS "external skipped (curl)")

endif(WITH_CURL)
