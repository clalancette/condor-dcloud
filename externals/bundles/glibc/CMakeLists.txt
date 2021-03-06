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


##############################################################
## Abandon all hope ye who enter here:
## this be the mystical workings of the infamous
## standard universe.  No moral platitudes apply, so
## batton your hatches, find yourself a cup of ale,
## because you are about to loose your mind.
## YARRR!
##############################################################

if (NOT CLIPPED)

	# check only the major and minor bits of gcc version.
	string(SUBSTRING ${CMAKE_C_COMPILER_VERSION} 0 2 GCC_VER_CHECK)
	# corner off glibc b/c it effect  
	set(GLIBC_PREFIX ${EXTERNAL_STAGE}/opt/glibc/)
	set(GLIBC_DETECTED ON)

	if (${GLIBC_VERSION} STREQUAL "23")
		set (VER 2.3.2.27.9.7)

		set (GLIBC_PATCH  patch --binary -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/${VER}/x86_64-syscall.S.patch )

		if (${GCC_VER_CHECK} MATCHES "3.4")
			set (GLIBC_PATCH  ${GLIBC_PATCH} && patch --binary -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/${VER}/gcc-3.4-fixes.patch )
		elseif (${GCC_VER_CHECK} MATCHES "4.")
			set (GLIBC_PATCH  ${GLIBC_PATCH} && patch --binary -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/${VER}/gcc-4-fixes.patch )
		endif()

		set (GLIBC_CONFIGURE ./configure --enable-static-nss --enable-add-ons=linuxthreads,c_stubs,glibc-compat --disable-shared --prefix=${GLIBC_PREFIX} --without-cvs)

		set (GLIBC_MAKE make )

	elseif(${GLIBC_VERSION} STREQUAL "25")

		if (${SYS_ARCH} STREQUAL "X86_64")

			set (VER 2.5-20061008T1257-x86_64-p0)

			set (GLIBC_CONFIGURE ./configure --prefix=${GLIBC_PREFIX} --enable-add-ons=nptl,rtkaio,c_stubs,libidn --without-cvs --enable-kernel=2.6.9 --with-headers=$HOME/junk/glibc-2.5-20061008T1257/override_headers:/usr/include --enable-bind-now --with-tls --with-__thread --build x86_64-redhat-linux --host x86_64-redhat-linux --disable-profile --enable-static-nss)

			set (GLIBC_MAKE make -j2 -r 'CFLAGS=-mtune=generic -DNDEBUG=1 -g -O3 ' PARALLELMFLAGS=-s)

		else()
			set (VER 2.5-20061008T1257-p0)

			set (GLIBC_CONFIGURE ./configure --prefix=${GLIBC_PREFIX} --enable-add-ons=nptl,rtkaio,c_stubs,libidn --without-cvs --enable-kernel=2.6.9 --with-headers=$HOME/junk/glibc-2.5-20061008T1257/override_headers:/usr/include --enable-bind-now --with-tls --with-__thread --build i486-redhat-linux --host i486-redhat-linux --disable-profile --enable-static-nss)

			set (GLIBC_MAKE make -j2 -r 'CFLAGS=-march=i386 -mtune=generic -mno-tls-direct-seg-refs -DNDEBUG=1 -g -O3 ' PARALLELMFLAGS=-s )
		endif()

		set (GLIBC_PATCH patch --binary -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/${VER}/enable-static-nss.patch && patch --binary -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/${VER}/disable-nscd.patch )

	elseif(${GLIBC_VERSION} STREQUAL "27")

		if (${SYS_ARCH} STREQUAL "X86_64")
			set (VER 2.7-18-x86_64)

			set (GLIBC_FLAGS CC=gcc-4.3 CXX=g++-4.3 AUTOCONF=false MAKEINFO=: CFLAGS='-pipe -O2 -fstrict-aliasing -g ')

            set (GLIBC_CONFIGURE ./configure --host=x86_64-linux-gnu --build=x86_64-linux-gnu --prefix=${GLIBC_PREFIX} --without-cvs --enable-add-ons=libidn,nptl --without-selinux --enable-kernel=2.6.8 --disable-profile --with-headers=${EXTERNAL_BUILD_PREFIX}/glibc-${VER}/src/glibc/glibc-${VER}/debian/include --enable-bind-now --with-tls --with-__thread --enable-static-nss)

			set (GLIBC_MAKE make -j2 -r)
		else()
			set (VER 2.7-18)

			set (GLIBC_FLAGS CC=gcc-4.3 CXX=g++-4.3 AUTOCONF=false MAKEINFO=: CFLAGS='-pipe -O2 -fstrict-aliasing -g -march=i686 -mtune=i686 -O3 ')

			set (GLIBC_CONFIGURE ./configure --host=i686-linux --build=i486-linux-gnu --prefix=${GLIBC_PREFIX} --without-cvs --enable-add-ons=libidn,nptl --without-selinux --enable-kernel=2.6.8 --disable-profile --with-headers=${EXTERNAL_BUILD_PREFIX}/glibc-${VER}/src/glibc/glibc-${VER}/debian/include --enable-bind-now --with-tls --with-__thread --enable-static-nss)

			set (GLIBC_MAKE make -j2 -r 'CFLAGS=-pipe -O2 -fstrict-aliasing -g -march=i686 -mtune=i686 -O3 ' PARALLELMFLAGS=-s)
		endif()

		set (GLIBC_PATCH patch --binary -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/${VER}/enable-static-nss.patch && patch --binary -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/${VER}/disable-nscd.patch )

	else()
		message(STATUS "Condor can not build standard universe for glibc version:${GLIBC_VERSION}")
		set(GLIBC_DETECTED OFF)
		set(CLIPPED ON PARENT_SCOPE )
	endif()

	if (GLIBC_DETECTED)
	  set (GLIBC_VER glibc-${VER})
	  set (GLIBC_INSTALL make install )

	  ExternalProject_Add(glibc
					  PREFIX ${EXTERNAL_BUILD_PREFIX}/${GLIBC_VER}
					  #-- Download Step ----------
					  DOWNLOAD_COMMAND wget -N
					  http://parrot.cs.wisc.edu/externals/${GLIBC_VER}.tar.gz ${CMD_TERM}
					  DOWNLOAD_DIR ${EXTERNAL_DL}
					  URL http://parrot.cs.wisc.edu/externals/${GLIBC_VER}.tar.gz
					  #--Patch step ----------
					  PATCH_COMMAND cd ${GLIBC_VER} && ${GLIBC_PATCH}
					  #--Configure step ----------
					  CONFIGURE_COMMAND ${GLIBC_FLAGS} ${GLIBC_VER}/${GLIBC_CONFIGURE}
					  #--Build Step ----------
					  BUILD_COMMAND ${GLIBC_MAKE}
					  BUILD_IN_SOURCE 1
					  #--install Step ----------
					  INSTALL_DIR ${EXTERNAL_STAGE}
					  INSTALL_COMMAND ${GLIBC_INSTALL} )

	  set( EXT_GLIBC_FOUND "${GLIBC_PREFIX}/lib/libc.a" )
	  message ( STATUS "external configured (EXT_GLIBC_FOUND=${EXT_GLIBC_FOUND})" )
	  set( EXT_GLIBC_FOUND ${EXT_GLIBC_FOUND} PARENT_SCOPE )
	  append_var(CONDOR_EXTERNALS glibc)

	  install ( FILES ${GLIBC_PREFIX}/lib/libc.a DESTINATION ${C_LIB} RENAME libcondor_c.a)
	  install ( FILES ${GLIBC_PREFIX}/lib/libnss_files.a DESTINATION ${C_LIB} RENAME libcondor_nss_files.a)
	  install ( FILES ${GLIBC_PREFIX}/lib/libnss_dns.a DESTINATION ${C_LIB} RENAME libcondor_nss_dns.a)
	  install ( FILES ${GLIBC_PREFIX}/lib/libresolv.a DESTINATION ${C_LIB} RENAME libcondor_resolv.a)
  endif(GLIBC_DETECTED)

endif(NOT CLIPPED)
