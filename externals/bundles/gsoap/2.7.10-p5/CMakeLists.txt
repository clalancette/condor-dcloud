option(WITH_GSOAP "Compiling with support for GSOAP" ON)

if (WITH_GSOAP)

	if ( NOT PROPER )

		set (GSOAP_VER gsoap-2.7.10)

		if (WINDOWS)
			set (GSOAP_PATCH cp ${CMAKE_CURRENT_SOURCE_DIR}/config.WINDOWS.h gsoap/src/config.h)
			
			set (GSOAP_CONFIGURE cp ${CMAKE_CURRENT_SOURCE_DIR}/Makefile.win32 gsoap/src/Makefile.win32 )
			
			set (GSOAP_MAKE set BISON_SIMPLE=${BISON_SIMPLE} \r\n
			                gmake -f Makefile.win32 -C gsoap/src )
			
			set (GSOAP_INSTALL set PACKAGE_INSTALL_DIR=${EXTERNAL_STAGE} \r\n
			                   gmake -f Makefile.win32 install -C gsoap/src )
		else(WINDOWS)
			set (GSOAP_PATCH  patch --binary -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/install_soapcpp2_wsdl2h_aux-2.7.10-Makefile.in.patch &&
							  patch --binary -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/fix_xlocale_insanity.patch &&
							  patch --binary -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/nmi_prereq_hack.patch)
			set (GSOAP_CONFIGURE ./configure --prefix=${EXTERNAL_STAGE} --disable-dependency-tracking CPPFLAGS=-I${EXTERNAL_STAGE}/include/)
			set (GSOAP_MAKE make )
			set (GSOAP_INSTALL make install )
		endif(WINDOWS)

		ExternalProject_Add( gsoap
					DEPENDS openssl
				    PREFIX ${EXTERNAL_BUILD_PREFIX}/${GSOAP_VER}
				    #-- Download Step ----------
				    DOWNLOAD_COMMAND wget -N http://parrot.cs.wisc.edu/externals/${GSOAP_VER}.tar.gz ${CMD_TERM}
				    DOWNLOAD_DIR ${EXTERNAL_DL}
				    URL http://parrot.cs.wisc.edu/externals/${GSOAP_VER}.tar.gz
					#--Patch step ----------
					PATCH_COMMAND cd ${GSOAP_VER} ${CMD_TERM} ${GSOAP_PATCH}
					#--Configure step ----------
				    CONFIGURE_COMMAND cd ${GSOAP_VER} ${CMD_TERM} ${GSOAP_CONFIGURE}
				    #--Build Step ----------
				    BUILD_COMMAND cd ${GSOAP_VER} ${CMD_TERM} ${GSOAP_MAKE}
				    BUILD_IN_SOURCE 1
				    #--install Step ----------
				    INSTALL_DIR ${EXTERNAL_STAGE}
				    INSTALL_COMMAND cd ${GSOAP_VER} ${CMD_TERM} ${GSOAP_INSTALL} )

		# Set the target dependencies which the rest of condor depends on.
		if (NOT WINDOWS)
			set(GSOAP_FOUND "${EXTERNAL_STAGE}/lib/libgsoapssl++.a")
			set(SOAPCPP2 "${EXTERNAL_STAGE}/bin/soapcpp2")
		else(NOT WINDOWS)
			set(GSOAP_FOUND ON)
			set(GSOAP_SRC "${EXTERNAL_STAGE}/src/stdsoap2.cpp" PARENT_SCOPE )
			set(SOAPCPP2 "${EXTERNAL_STAGE}/soapcpp2.exe")
		endif(NOT WINDOWS)

		set ( GSOAP_SHARE "${EXTERNAL_BUILD_PREFIX}/${GSOAP_VER}/src/gsoap/${GSOAP_VER}/gsoap" )		
		append_var (CONDOR_EXTERNALS gsoap)

	else( NOT PROPER )

		#find gsoap and supporting bit(s)
		find_multiple( "gsoapssl++" GSOAP_FOUND)
		find_program( SOAPCPP2
					  soapcpp2
		    	      HINTS /usr/bin )

		set ( SOAPCPP2 ${SOAPCPP2} )
		set ( GSOAP_SHARE "/usr/share/gsoap" )

	endif( NOT PROPER )

	if (GSOAP_FOUND)
		message ( STATUS "external configured (GSOAP_FOUND=${GSOAP_FOUND})" )
		
		if (NOT WINDOWS)
			set( GSOAP_FOUND ${GSOAP_FOUND} PARENT_SCOPE )
		endif(NOT WINDOWS)
		
		set( SOAPCPP2 ${SOAPCPP2} PARENT_SCOPE )
		set( GSOAP_SHARE ${GSOAP_SHARE} PARENT_SCOPE )
		set( HAVE_EXT_GSOAP ON PARENT_SCOPE )

	endif (GSOAP_FOUND)

else(WITH_GSOAP)

	message (STATUS "external skiped (gsoap)")

endif(WITH_GSOAP)