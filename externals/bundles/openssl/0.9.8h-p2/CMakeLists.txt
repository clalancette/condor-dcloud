option(WITH_OPENSSL "Compiling with support for OPENSSL" ON)

if (WITH_OPENSSL)

	if ( NOT PROPER )

		set(OPENSSL_VER openssl-0.9.8h)

		if (WINDOWS)
			set (OPENSSL_PATCH patch --binary -p0 -i ${CMAKE_CURRENT_SOURCE_DIR}/renegotiate.patch \r\n
							   patch --binary -p0 -i ${CMAKE_CURRENT_SOURCE_DIR}/mk1mf.pl-0.9.8h-patch \r\n
							   patch --binary -p0 -i ${CMAKE_CURRENT_SOURCE_DIR}/VC-32.pl-0.9.8h-patch )
			set (OPENSSL_CONFIGURE perl Configure VC-WIN32 )
			set (OPENSSL_MAKE  call ms/do_nt.bat \r\n
							   nmake -f ms/ntdll.mak )
			set (OPENSSL_INSTALL cp out32dll/*.lib ${EXTERNAL_STAGE}/lib \r\n
								 cp out32dll/*.dll ${EXTERNAL_STAGE}/lib \r\n
								 cp -r inc32/* ${EXTERNAL_STAGE}/include \r\n
								 cp ${CMAKE_CURRENT_SOURCE_DIR}/ossl_typ-post.h ${EXTERNAL_STAGE}/include/openssl/ossl_typ.h )
		else(WINDOWS)
			set (OPENSSL_PATCH patch --binary -p0 -i ${CMAKE_CURRENT_SOURCE_DIR}/renegotiate.patch &&
							   sed 's/-fomit-frame-pointer//' Configure > Configure.new &&
							   mv Configure.new Configure )
			set (OPENSSL_CONFIGURE ./config --prefix=${EXTERNAL_STAGE} no-asm )
			if ( NOT ${OS_NAME} MATCHES "AIX")
				set (OPENSSL_CONFIGURE ${OPENSSL_CONFIGURE} -fPIC)
			endif()
			set (OPENSSL_MAKE make && make test )
			set (OPENSSL_INSTALL make install )
		endif(WINDOWS)

		ExternalProject_Add( openssl
				    PREFIX ${EXTERNAL_BUILD_PREFIX}/${OPENSSL_VER}
				    #-- Download Step ----------
				    DOWNLOAD_COMMAND wget -N http://parrot.cs.wisc.edu/externals/${OPENSSL_VER}.tar.gz ${CMD_TERM}
				    DOWNLOAD_DIR ${EXTERNAL_DL}
				    URL http://parrot.cs.wisc.edu/externals/${OPENSSL_VER}.tar.gz
					#--Patch step ----------
					PATCH_COMMAND cd ${OPENSSL_VER} ${CMD_TERM} ${OPENSSL_PATCH}
				    #--Configure step ----------
				    CONFIGURE_COMMAND cd ${OPENSSL_VER} ${CMD_TERM} ${OPENSSL_CONFIGURE}
				    #--Build Step ----------
				    BUILD_COMMAND cd ${OPENSSL_VER} ${CMD_TERM} ${OPENSSL_MAKE}
				    BUILD_IN_SOURCE 1
				    #--install Step ----------
				    INSTALL_DIR ${EXTERNAL_STAGE}
				    INSTALL_COMMAND cd ${OPENSSL_VER} ${CMD_TERM} ${OPENSSL_INSTALL} )

		# Set the target dependencies which the rest of condor depends on.
		if (WINDOWS)
			set(OPENSSL_FOUND "${EXTERNAL_STAGE}/lib/libeay32.lib;${EXTERNAL_STAGE}/lib/ssleay32.lib")

			#this may become the common case.
			install ( FILES ${EXTERNAL_STAGE}/lib/libeay32.dll
							${EXTERNAL_STAGE}/lib/ssleay32.dll
					  DESTINATION ${C_LIB} )

		else(WINDOWS)
			set(OPENSSL_FOUND "${EXTERNAL_STAGE}/lib/libssl.a;${EXTERNAL_STAGE}/lib/libcrypto.a")
		endif(WINDOWS)

		append_var(CONDOR_EXTERNALS openssl)

	else( NOT PROPER )

		find_multiple( "ssl;crypto" OPENSSL_FOUND )

	endif( NOT PROPER )

	if (OPENSSL_FOUND)
		message (STATUS "external configured (OPENSSL_FOUND=${OPENSSL_FOUND})")
		set( OPENSSL_FOUND ${OPENSSL_FOUND} PARENT_SCOPE )
		set( HAVE_EXT_OPENSSL ON PARENT_SCOPE )
		set( HAVE_OPENSSL_SSL_H ON PARENT_SCOPE )
	endif(OPENSSL_FOUND)

else(WITH_OPENSSL)

	message (STATUS "external skiped (openssl)")

endif(WITH_OPENSSL)