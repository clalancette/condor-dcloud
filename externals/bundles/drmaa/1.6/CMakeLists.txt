option(WITH_DRMAA "Compiling with support for drmaa" ON)

if ( NOT PROPER AND WITH_DRMAA )

	set (DRMAA_VER drmaa-1.6)

	if (NOT WINDOWS)
		set (DRMAA_CONFIGURE ./configure --prefix=${EXTERNAL_STAGE})
		set (DRMAA_MAKE make)
		set (DRMAA_INSTALL cp drmaa.h ${EXTERNAL_STAGE}/include/ )

		if(DARWIN)
			set (DRMAA_INSTALL cp libdrmaa.dylib ${EXTERNAL_STAGE}/lib/ && ${DRMAA_INSTALL})
			set( DRMAA_FOUND "${EXTERNAL_STAGE}/lib/libdrmaa.dylib" )
		else()
			set (DRMAA_INSTALL cp libdrmaa.so ${EXTERNAL_STAGE}/lib/ && ${DRMAA_INSTALL})
			set( DRMAA_FOUND "${EXTERNAL_STAGE}/lib/libdrmaa.so" )
		endif()

	else()
		set (DRMAA_CONFIGURE echo "Nothing to configure for Windows")
		set (DRMAA_MAKE nmake /f libdrmaa.mak CFG="libdrmaa - Win32 Release")
		set (DRMAA_INSTALL  cp Release/* ${EXTERNAL_STAGE}/lib \r\n
		                    cp drmaa.h ${EXTERNAL_STAGE}/include)
			
		set( DRMAA_FOUND "${EXTERNAL_STAGE}/lib/libdrmaa.lib" )

		install ( FILES ${EXTERNAL_STAGE}/lib/libdrmaa.dll DESTINATION ${C_LIB} )
	endif()

	ExternalProject_Add(drmaa
						PREFIX ${EXTERNAL_BUILD_PREFIX}/${DRMAA_VER}
						#-- Download Step ----------
						DOWNLOAD_COMMAND wget -N http://parrot.cs.wisc.edu/externals/${DRMAA_VER}.tar.gz ${CMD_TERM}
						DOWNLOAD_DIR ${EXTERNAL_DL}
						URL http://parrot.cs.wisc.edu/externals/${DRMAA_VER}.tar.gz
						#--Configure step ----------
						CONFIGURE_COMMAND ${DRMAA_CONFIGURE}
						#--Build Step ----------
						BUILD_COMMAND ${DRMAA_MAKE}
						BUILD_IN_SOURCE 1
						#--install Step ----------
						INSTALL_DIR ${EXTERNAL_STAGE}
						INSTALL_COMMAND ${DRMAA_INSTALL})
	
	message (STATUS "external configured (DRMAA_FOUND=${DRMAA_FOUND})")
	set( DRMAA_FOUND ${DRMAA_FOUND} PARENT_SCOPE )
	set( HAVE_EXT_DRMAA ON PARENT_SCOPE )
	append_var (CONDOR_EXTERNALS drmaa)

	# i've tar bundled drmaa, does anyone actually use this?  if so I can explode. 
	install ( FILES ${EXTERNAL_DL}/${DRMAA_VER}.tar.gz DESTINATION ${C_SRC} )

else ()
	message ( STATUS "external skipped (drmaa)" )
endif ()




