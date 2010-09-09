option(WITH_LIBVIRT "Compiling with support for LIBVIRT" ON)

if (WITH_LIBVIRT)

	if ( NOT PROPER )

		set (LIBVERT_VER libvirt-0.6.2)

		ExternalProject_Add(libvirt
					DEPENDS libxml2
				    PREFIX ${EXTERNAL_BUILD_PREFIX}/${LIBVERT_VER}
				    #-- Download Step ----------
					DOWNLOAD_COMMAND wget -N http://parrot.cs.wisc.edu/externals/${LIBVERT_VER}.tar.gz ${CMD_TERM}
				    DOWNLOAD_DIR ${EXTERNAL_DL}
				    URL http://parrot.cs.wisc.edu/externals/${LIBVERT_VER}.tar.gz
					PATCH_COMMAND cd ${LIBVERT_VER} && patch -p0 -i ${CMAKE_CURRENT_SOURCE_DIR}/no_gnutls.patch
					#--Configure step ----------
				    CONFIGURE_COMMAND cd ${LIBVERT_VER} && ./configure --prefix=${EXTERNAL_STAGE} --disable-shared --without-python --without-xen --without-libvirtd --without-remote --without-test --without-uml --without-openvz --without-avahi --without-selinux --without-sasl --with-libxml=${EXTERNAL_STAGE}
				    #--Build Step ----------
				    BUILD_COMMAND cd ${LIBVERT_VER} && make
				    BUILD_IN_SOURCE 1
				    #--install Step ----------
				    INSTALL_DIR ${EXTERNAL_STAGE}
				    INSTALL_COMMAND cd ${LIBVERT_VER} && make install )

		# Set the target dependencies which the rest of condor depends on.
		set(LIBVIRT_FOUND "${EXTERNAL_STAGE}/lib/libvirt.a")
		append_var(CONDOR_EXTERNALS libvirt)

	else( NOT PROPER )

		find_multiple( "virt" LIBVIRT_FOUND )

	endif( NOT PROPER )

	if (LIBVIRT_FOUND)
		message (STATUS "external configured (LIBVIRT_FOUND=${LIBVIRT_FOUND})")
		set( LIBVIRT_FOUND ${LIBVIRT_FOUND} PARENT_SCOPE )
		set( HAVE_EXT_LIBVIRT ON PARENT_SCOPE )
	endif(LIBVIRT_FOUND)

else(WITH_LIBVIRT)

	message (STATUS "external skiped (libvirt)")

endif(WITH_LIBVIRT)