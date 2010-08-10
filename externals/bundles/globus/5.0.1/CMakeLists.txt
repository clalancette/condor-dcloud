option(WITH_GLOBUS "Compiling with support for GLOBUS" ON)

if (WITH_GLOBUS)

	if (NOT PROPER)
		set (GT4_VER globus-4.0.8 )
		set (GT42_VER globus-4.2.1 )
		set (GLOBUS_VER globus-5.0.1)

		ExternalProject_Add(gt4
					PREFIX ${EXTERNAL_BUILD_PREFIX}/${GT4_VER}
					#-- Download Step ----------
					DOWNLOAD_COMMAND wget -N http://parrot.cs.wisc.edu/externals/${GT4_VER}-java.tar.gz ${CMD_TERM}
					DOWNLOAD_DIR ${EXTERNAL_DL}
					URL http://parrot.cs.wisc.edu/externals/${GT4_VER}-java.tar.gz
					#--Configure step ----------
					CONFIGURE_COMMAND echo "Nothing to do"
					#--Build Step ----------
					BUILD_COMMAND echo "Nothing to do"
					BUILD_IN_SOURCE 1
					#--install Step ----------
					INSTALL_DIR ${EXTERNAL_STAGE}
					INSTALL_COMMAND cp -r gt4-java ${EXTERNAL_STAGE}/${C_LIB}/gt4 )

		ExternalProject_Add(gt42
							PREFIX ${EXTERNAL_BUILD_PREFIX}/${GT42_VER}
							#-- Download Step ----------
							DOWNLOAD_COMMAND wget -N http://parrot.cs.wisc.edu/externals/${GT42_VER}-java.tar.gz ${CMD_TERM}
							DOWNLOAD_DIR ${EXTERNAL_DL}
							URL http://parrot.cs.wisc.edu/externals/${GT42_VER}-java.tar.gz
							#--Configure step ----------
							CONFIGURE_COMMAND echo "Nothing to do"
							#--Build Step ----------
							BUILD_COMMAND echo "Nothing to do"
							BUILD_IN_SOURCE 1
							#--install Step ----------
							INSTALL_DIR ${EXTERNAL_STAGE}
							INSTALL_COMMAND cp -r gt42-java ${EXTERNAL_STAGE}/${C_LIB}/gt42 )

		set (GLOBUS_PATCH patch -i ${CMAKE_CURRENT_SOURCE_DIR}/xio-plugins.patch -p0 &&
						cd gt5.0.1-all-source-installer/source-trees/xio/src/builtins &&
						mkdir -p gsi popen &&
						sed -e 's/globus_xio_gsi.h/globus_xio_gsi_driver.h/' ../../drivers/gsi/globus_i_xio_gsi.h >gsi/globus_i_xio_gsi.h &&
						cp ../../drivers/gsi/globus_xio_gsi.c gsi/globus_xio_gsi_driver.c &&
						cp ../../drivers/gsi/globus_xio_gsi.h gsi/globus_xio_gsi_driver.h &&
						cp ../../drivers/popen/source/*.h popen &&
						cp ../../drivers/popen/source/*.c popen &&
						sed -e 's/tcp/gsi/g' tcp/Makefile.am >gsi/Makefile.am &&
						sed -e 's/tcp/gsi/g' tcp/Makefile.in >gsi/Makefile.in &&
						sed -e 's/tcp/popen/g' tcp/Makefile.am >popen/Makefile.am &&
						sed -e 's/tcp/popen/g' tcp/Makefile.in >popen/Makefile.in &&
						cd ../../../../source-trees-thr/xio/src/builtins &&
						mkdir -p gsi popen &&
						cd gsi &&
						ln -f -s ../../../../../source-trees/xio/src/builtins/gsi/Makefile.am . &&
						ln -f -s ../../../../../source-trees/xio/src/builtins/gsi/Makefile.in . &&
						ln -f -s ../../../../../source-trees/xio/src/builtins/gsi/globus_i_xio_gsi.h . &&
						ln -f -s ../../../../../source-trees/xio/src/builtins/gsi/globus_xio_gsi_driver.c . &&
						ln -f -s ../../../../../source-trees/xio/src/builtins/gsi/globus_xio_gsi_driver.h . &&
						cd ../popen &&
						ln -f -s ../../../../../source-trees/xio/src/builtins/popen/Makefile.am . &&
						ln -f -s ../../../../../source-trees/xio/src/builtins/popen/Makefile.in . &&
						ln -f -s ../../../../../source-trees/xio/src/builtins/popen/globus_xio_popen_driver.c . &&
						ln -f -s ../../../../../source-trees/xio/src/builtins/popen/globus_xio_popen_driver.h . )

		if(${OS_NAME} MATCHES "SUNOS")
			set ( GLOBUS_PATCH patch -i ${CMAKE_CURRENT_SOURCE_DIR}/putenv.patch -p0 && ${GLOBUS_PATCH} )
		elseif(${OS_NAME} MATCHES "DARWIN")
			set ( GLOBUS_PATCH ${GLOBUS_PATCH} && patch -i  ${CMAKE_CURRENT_SOURCE_DIR}/xio-plugins-2.patch -p0 )
		endif()

		set (GLOBUS_FLAGS CFLAGS=-I${EXTERNAL_STAGE}/include && export CFLAGS &&
						LDFLAGS=-L${EXTERNAL_STAGE}/lib && export LDFLAGS &&
						OPENSSL_CFLAGS=-I${EXTERNAL_STAGE}/include && export OPENSSL_CFLAGS &&
						OPENSSL_LDFLAGS=-L${EXTERNAL_STAGE}/lib && export OPENSSL_LDFLAGS  )

		if( ${OS_NAME} MATCHES "SUNOS" OR ${OS_NAME} MATCHES "LINUX")
			set (GLOBUS_FLAGS ${GLOBUS_FLAGS} && OPENSSL_LIBS='-lssl -lcrypto -ldl' && export OPENSSL_LIBS)
		elseif(${OS_NAME} MATCHES "DARWIN")
			set (GLOBUS_FLAGS ${GLOBUS_FLAGS} &&
							OPENSSL_LIBS="-lssl -lcrypto" && export OPENSSL_LIBS &&
							OPENSSL_LDFLAGS="-L${EXTERNAL_STAGE}/lib -Wl,-search_paths_first" && export OPENSSL_LDFLAGS)
		endif()

		set (GLOBUS_DEPENDS openssl gt4 gt42)
		if (KRB5_FOUND)
			set (GLOBUS_DEPENDS ${GLOBUS_DEPENDS} krb5)
		endif(KRB5_FOUND)

		if(${SYS_ARCH} MATCHES "IA64|X86_64")
			set( GLOBUS_FLAVOR gcc64dbg )
		elseif(${OS_NAME} MATCHES "AIX")
			set( GLOBUS_FLAVOR vendorcc32dbg )
		else ()
			set( GLOBUS_FLAVOR gcc32dbg )
		endif()

		# honestly the build of globus is one of the strangest things I have ever seen and it makes no sense to me.
		ExternalProject_Add(globus
							DEPENDS ${GLOBUS_DEPENDS}
							PREFIX ${EXTERNAL_BUILD_PREFIX}/${GLOBUS_VER}
							#-- Download Step ----------
							DOWNLOAD_COMMAND wget -N http://parrot.cs.wisc.edu/externals/${GLOBUS_VER}.tar.gz ${CMD_TERM}
							DOWNLOAD_DIR ${EXTERNAL_DL}
							URL http://parrot.cs.wisc.edu/externals/${GLOBUS_VER}.tar.gz
							#--Patch step ----------
							PATCH_COMMAND ${GLOBUS_PATCH}
							#--Configure step ----------
							CONFIGURE_COMMAND cd gt5.0.1-all-source-installer &&
								${GLOBUS_FLAGS} &&
							./configure --prefix=${EXTERNAL_STAGE} --with-flavor=${GLOBUS_FLAVOR}pthr &&
								make gpt gssapi_error globus-resource-management-sdk
							#--Build Step ----------
							BUILD_COMMAND cd gt5.0.1-all-source-installer &&
								${GLOBUS_FLAGS} &&
								./configure --prefix=${EXTERNAL_STAGE} --with-flavor=${GLOBUS_FLAVOR} &&
								make gpt gssapi_error globus-resource-management-sdk &&
								GLOBUS_LOCATION=${EXTERNAL_STAGE} && export GLOBUS_LOCATION &&
								make globus_gridftp_server &&
								${EXTERNAL_STAGE}/sbin/gpt-build -force -static -srcdir=source-trees/gridftp/server/src ${GLOBUS_FLAVOR} &&
								mv ${EXTERNAL_STAGE}/sbin/globus-gridftp-server ${EXTERNAL_STAGE}/sbin/globus-gridftp-server.dynamic
							BUILD_IN_SOURCE 1
							#--install Step ----------
							INSTALL_DIR ${EXTERNAL_STAGE}
							INSTALL_COMMAND echo "done with the craziness that is globus" )

			## Set all the config information
			set (GLOBUS_FOUND "${EXTERNAL_STAGE}/lib/libglobus_gss_assist_${GLOBUS_FLAVOR}.a;${EXTERNAL_STAGE}/lib/libglobus_gssapi_gsi_${GLOBUS_FLAVOR}.a;${EXTERNAL_STAGE}/lib/libglobus_gsi_proxy_core_${GLOBUS_FLAVOR}.a;${EXTERNAL_STAGE}/lib/libglobus_gsi_credential_${GLOBUS_FLAVOR}.a;${EXTERNAL_STAGE}/lib/libglobus_gsi_callback_${GLOBUS_FLAVOR}.a;${EXTERNAL_STAGE}/lib/libglobus_gsi_sysconfig_${GLOBUS_FLAVOR}.a;${EXTERNAL_STAGE}/lib/libglobus_oldgaa_${GLOBUS_FLAVOR}.a;${EXTERNAL_STAGE}/lib/libglobus_gsi_cert_utils_${GLOBUS_FLAVOR}.a;${EXTERNAL_STAGE}/lib/libglobus_openssl_${GLOBUS_FLAVOR}.a;${EXTERNAL_STAGE}/lib/libglobus_openssl_error_${GLOBUS_FLAVOR}.a;${EXTERNAL_STAGE}/lib/libglobus_proxy_ssl_${GLOBUS_FLAVOR}.a;${EXTERNAL_STAGE}/lib/libglobus_callout_${GLOBUS_FLAVOR}.a;${EXTERNAL_STAGE}/lib/libglobus_common_${GLOBUS_FLAVOR}.a;${EXTERNAL_STAGE}/lib/libltdl_${GLOBUS_FLAVOR}.a")

			set (GLOBUS_GRID_UNIVERSE_COMMON
			"${EXTERNAL_STAGE}/lib/libglobus_gssapi_error_${GLOBUS_FLAVOR}.a;${EXTERNAL_STAGE}/lib/libglobus_xio_${GLOBUS_FLAVOR}.a;${EXTERNAL_STAGE}/lib/libglobus_io_${GLOBUS_FLAVOR}.a;${EXTERNAL_STAGE}/lib/libglobus_rsl_${GLOBUS_FLAVOR}.a")
			
			set (GLOBUS_GRID_UNIVERSE_GT2
			"${EXTERNAL_STAGE}/lib/libglobus_gass_transfer_${GLOBUS_FLAVOR}.a;${EXTERNAL_STAGE}/lib/libglobus_gram_client_${GLOBUS_FLAVOR}.a;${EXTERNAL_STAGE}/lib/libglobus_gram_protocol_${GLOBUS_FLAVOR}.a")
			set (GLOBUS_GRID_UNIVERSE_NORUID
			"${EXTERNAL_STAGE}/lib/libglobus_ftp_client_${GLOBUS_FLAVOR}.a;${EXTERNAL_STAGE}/lib/libglobus_ftp_control_${GLOBUS_FLAVOR}.a")
			append_var (CONDOR_EXTERNALS globus)

			install ( DIRECTORY ${EXTERNAL_STAGE}/${C_LIB}/gt4 DESTINATION ${C_LIB} )
			install ( DIRECTORY ${EXTERNAL_STAGE}/${C_LIB}/gt42 DESTINATION ${C_LIB} )

	else( NOT PROPER )

		find_multiple( "globus_gss_assist;globus_gssapi_gsi;globus_gsi_proxy_core;globus_gsi_credential;globus_gsi_callback;globus_gsi_sysconfig;globus_oldgaa;globus_gsi_cert_utils;globus_openssl;globus_openssl_error;globus_proxy_ssl;globus_callout;globus_common;ltdl" GLOBUS_FOUND )
		find_multiple( "globus_gssapi_error;globus_xio;globus_io;globus_rsl" GLOBUS_GRID_UNIVERSE_COMMON )
		find_multiple( "globus_gass_transfer;globus_gram_client;globus_gram_protocol" GLOBUS_GRID_UNIVERSE_GT2 )
		find_multiple( "globus_ftp_client;globus_ftp_control" GLOBUS_GRID_UNIVERSE_NORUID)

	endif(NOT PROPER)

	if (GLOBUS_FOUND)

		message( STATUS "external configured (GLOBUS_FOUND=${GLOBUS_FOUND})" )
		set( HAVE_EXT_GLOBUS ON PARENT_SCOPE )
		set( GLOBUS_FLAVOR ${GLOBUS_FLAVOR} PARENT_SCOPE )
		set( GLOBUS_FOUND ${GLOBUS_FOUND} PARENT_SCOPE )

		if (GLOBUS_GRID_UNIVERSE_COMMON)
			message( STATUS "(GLOBUS_GRID_UNIVERSE_COMMON=${GLOBUS_GRID_UNIVERSE_COMMON})" )
			set( GLOBUS_GRID_UNIVERSE_COMMON ${GLOBUS_GRID_UNIVERSE_COMMON} PARENT_SCOPE )
		endif(GLOBUS_GRID_UNIVERSE_COMMON)

		if (GLOBUS_GRID_UNIVERSE_GT2)
			message( STATUS "(GLOBUS_GRID_UNIVERSE_GT2=${GLOBUS_GRID_UNIVERSE_GT2})" )
			set( GLOBUS_GRID_UNIVERSE_GT2 ${GLOBUS_GRID_UNIVERSE_GT2} PARENT_SCOPE )
		endif(GLOBUS_GRID_UNIVERSE_GT2)

		if (GLOBUS_GRID_UNIVERSE_NORUID)
			message( STATUS "(GLOBUS_GRID_UNIVERSE_NORUID=${GLOBUS_GRID_UNIVERSE_NORUID})" )
			set( GLOBUS_GRID_UNIVERSE_NORUID ${GLOBUS_GRID_UNIVERSE_NORUID} PARENT_SCOPE )
		endif(GLOBUS_GRID_UNIVERSE_NORUID)

	endif(GLOBUS_FOUND)

else( WITH_GLOBUS )

	message (STATUS "external skiped (globus)")

endif( WITH_GLOBUS )
