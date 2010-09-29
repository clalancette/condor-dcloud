#option is defaultly disabled.
option( WITH_CREAM "Compiling with support for CREAM" OFF )

if ( NOT PROPER AND WITH_CREAM AND LINUX )

		# ok, much like globus the is a goble-dy-guk monstrosity that needs to die.
		dprint("CREAM BUILD IS UNHOLY!!!")

		# i feel dirty
		set (CREAM_STAGE ${EXTERNAL_STAGE}/opt/cream)
		
		set (GSOAP_VER gsoap_2.7.6b)
		ExternalProject_Add(cream_soap
				    PREFIX ${EXTERNAL_BUILD_PREFIX}/${GSOAP_VER}
				    #-- Download Step ----------
				    DOWNLOAD_COMMAND wget -N http://parrot.cs.wisc.edu/externals/${GSOAP_VER}.tar.gz ${CMD_TERM}
				    DOWNLOAD_DIR ${EXTERNAL_DL}
				    URL http://parrot.cs.wisc.edu/externals/${GSOAP_VER}.tar.gz
				    #--Configure step ----------
				    CONFIGURE_COMMAND cd gsoap-2.7 && ./configure --prefix=${CREAM_STAGE}
				    #--Build Step ----------
				    BUILD_COMMAND cd gsoap-2.7 && make
				    BUILD_IN_SOURCE 1
				    #--install Step ----------
				    INSTALL_DIR ${CREAM_STAGE}
				    INSTALL_COMMAND cd gsoap-2.7 && make install && cp soapcpp2/wsdl/stlvector.h ${CREAM_STAGE}/include )

		# wrong on so many levels
		set (BOOST_VER boost_1_33_1)
		ExternalProject_Add(boost
				    PREFIX ${EXTERNAL_BUILD_PREFIX}/${BOOST_VER}
				    #-- Download Step ----------
					DOWNLOAD_COMMAND wget -N http://parrot.cs.wisc.edu/externals/${BOOST_VER}.tar.gz ${CMD_TERM}
				    DOWNLOAD_DIR ${EXTERNAL_DL}
				    URL http://parrot.cs.wisc.edu/externals/${BOOST_VER}.tar.gz
				    #--Configure step ----------
				    CONFIGURE_COMMAND cd ${BOOST_VER}/tools/build/jam_src && 
				    	./build.sh &&
					mv `find . | grep bjam` ../../../
				    #--Build Step ----------
				    BUILD_COMMAND cd ${BOOST_VER} && bjam -sNO_COMPRESSION=1 -sTOOLS=gcc --prefix=${CREAM_STAGE} install 
				    BUILD_IN_SOURCE 1
				    #--install Step ----------
				    INSTALL_DIR ${CREAM_STAGE}
				    INSTALL_COMMAND cd ${CREAM_STAGE}/include && ln -s -f boost-1_33_1/boost . )

		set (LOG4CPP_VER log4cpp-1.0-3)
		ExternalProject_Add(log4cpp
				    PREFIX ${EXTERNAL_BUILD_PREFIX}/${LOG4CPP_VER}
				    #-- Download Step ----------
					DOWNLOAD_COMMAND wget -N http://parrot.cs.wisc.edu/externals/${LOG4CPP_VER}.tar.gz ${CMD_TERM}
				    DOWNLOAD_DIR ${EXTERNAL_DL}
				    URL http://parrot.cs.wisc.edu/externals/${LOG4CPP_VER}.tar.gz
				    #--Configure step ----------
				    CONFIGURE_COMMAND cd log4cpp-1.0 && ./configure --prefix=${CREAM_STAGE}
				    #--Build Step ----------
				    BUILD_COMMAND cd log4cpp-1.0 && make && make check
				    BUILD_IN_SOURCE 1
				    #--install Step ----------
				    INSTALL_DIR ${CREAM_STAGE}
				    INSTALL_COMMAND cd log4cpp-1.0 && make install )

		# build gridsite
		set (GRIDSITE_VER gridsite-1.6.0)
		ExternalProject_Add(gridsite
				    PREFIX ${EXTERNAL_BUILD_PREFIX}/${GRIDSITE_VER}
				    #-- Download Step ----------
				    DOWNLOAD_COMMAND wget -N http://parrot.cs.wisc.edu/externals/${GRIDSITE_VER}.src.tar.gz ${CMD_TERM}
				    DOWNLOAD_DIR ${EXTERNAL_DL}
				    URL http://parrot.cs.wisc.edu/externals/${GRIDSITE_VER}.src.tar.gz
				    #--Configure step ----------
				    CONFIGURE_COMMAND echo "no config"
				    #--Build Step ----------
				    BUILD_COMMAND cd org.gridsite.core/src && make libgridsite.a
				    BUILD_IN_SOURCE 1
				    #--install Step ----------
				    INSTALL_DIR ${CREAM_STAGE}
				    INSTALL_COMMAND cd org.gridsite.core/src && cp libgridsite.a ${CREAM_STAGE}/lib && cp ../interface/*.h ${CREAM_STAGE}/include )

		#build ares
		ExternalProject_Add(ares
				    PREFIX ${EXTERNAL_BUILD_PREFIX}/ares-1.1.1
				    #-- Download Step ----------
					DOWNLOAD_COMMAND wget -N http://parrot.cs.wisc.edu/externals/ares.tar.gz ${CMD_TERM}
				    DOWNLOAD_DIR ${EXTERNAL_DL}
				    URL http://parrot.cs.wisc.edu/externals/ares.tar.gz
				    #--Configure step ----------
				    CONFIGURE_COMMAND cd ares-1.1.1 && ./configure --prefix=${CREAM_STAGE}
				    #--Build Step ----------
				    BUILD_COMMAND cd ares-1.1.1 && make
				    BUILD_IN_SOURCE 1
				    #--install Step ----------
				    INSTALL_DIR ${CREAM_STAGE}
				    INSTALL_COMMAND cd ares-1.1.1 && make install )

		# build gsoap plugin.
		set (CSP_CC_FLAGS -I. -I../interface -DWITH_NONAMESPACES -I${CREAM_STAGE}/include -D_GNU_SOURCE -I${EXTERNAL_STAGE}/include/${GLOBUS_FLAVOR} -fPIC -DPIC)
		ExternalProject_Add(cream_soap_plugin
					DEPENDS globus
				    PREFIX ${EXTERNAL_BUILD_PREFIX}/cream_soap_plugin
				    #-- Download Step ----------
					DOWNLOAD_COMMAND wget -N http://parrot.cs.wisc.edu/externals/org.glite.security.gsoap-plugin.tar.gz ${CMD_TERM}
				    DOWNLOAD_DIR ${EXTERNAL_DL}
				    URL http://parrot.cs.wisc.edu/externals/org.glite.security.gsoap-plugin.tar.gz
				    #--Configure step ----------
				    CONFIGURE_COMMAND cd org.glite.security.gsoap-plugin/src &&
				    	${CMAKE_CURRENT_SOURCE_DIR}/cream_soap_plugin_patch.sh
				    #--Build Step ----------
				    BUILD_COMMAND cd org.glite.security.gsoap-plugin/src &&
						gcc ${CSP_CC_FLAGS} -o glite_gss.thr.o -c glite_gss.c &&
						gcc ${CSP_CC_FLAGS} -o glite_gsplugin.thr.o -c glite_gsplugin.c &&
						gcc ${CSP_CC_FLAGS} -o stdsoap2.thr.o -c stdsoap2.c &&
						ar crv libglite_security_gsoap_plugin_${GLOBUS_FLAVOR}pthr.a glite_gss.thr.o glite_gsplugin.thr.o stdsoap2.thr.o &&
						ranlib libglite_security_gsoap_plugin_${GLOBUS_FLAVOR}pthr.a
				    BUILD_IN_SOURCE 1
				    #--install Step ----------
				    INSTALL_DIR ${CREAM_STAGE}/lib
				    INSTALL_COMMAND  cd org.glite.security.gsoap-plugin/src &&
						cp libglite_security_gsoap_plugin_${GLOBUS_FLAVOR}pthr.a ${CREAM_STAGE}/lib &&
						cp stdsoap2.h ${CREAM_STAGE}/include &&
						mkdir -p ${CREAM_STAGE}/include/glite/security &&
						cp ../interface/*.h ${CREAM_STAGE}/include/glite/security
				)

		ExternalProject_Add(cream_wsdl
				    PREFIX ${EXTERNAL_BUILD_PREFIX}/cream_wsdl
				    #-- Download Step ----------
					DOWNLOAD_COMMAND wget -N http://parrot.cs.wisc.edu/externals/org.glite.ce.wsdl.tar.gz ${CMD_TERM}
				    DOWNLOAD_DIR ${EXTERNAL_DL}
				    URL http://parrot.cs.wisc.edu/externals/org.glite.ce.wsdl.tar.gz
				    #--Configure step ----------
				    CONFIGURE_COMMAND echo "nothing to do"
				    #--Build Step ----------
				    BUILD_COMMAND echo "nothing to do"
				    BUILD_IN_SOURCE 1
				    #--install Step ----------
				    INSTALL_DIR ${CREAM_STAGE}
				    INSTALL_COMMAND cd org.glite.ce.wsdl && project/install.sh ${CREAM_STAGE} ${CREAM_STAGE} 1.10.1 2.0.0 )

		ExternalProject_Add(cream
					DEPENDS classads voms cream_soap boost log4cpp gridsite ares cream_soap_plugin cream_wsdl
				    PREFIX ${EXTERNAL_BUILD_PREFIX}/cream
				    #-- Download Step ----------
					DOWNLOAD_COMMAND wget -N http://parrot.cs.wisc.edu/externals/org.glite.ce.cream-client-api-c.tar.gz ${CMD_TERM}
				    DOWNLOAD_DIR ${EXTERNAL_DL}
				    URL http://parrot.cs.wisc.edu/externals/org.glite.ce.cream-client-api-c.tar.gz
					#--Patch step ----------
					PATCH_COMMAND patch -p0 -i ${CMAKE_CURRENT_SOURCE_DIR}/cream.patch && 
						cp ${CMAKE_CURRENT_SOURCE_DIR}/*.m4 org.glite.ce.cream-client-api-c/project
				    #--Configure step ----------
				    CONFIGURE_COMMAND cd org.glite.ce.cream-client-api-c &&
						./bootstrap &&
						./configure --prefix=${CREAM_STAGE} --with-globus-prefix=${EXTERNAL_STAGE} --with-globus-thr-flavor=${GLOBUS_FLAVOR}pthr --with-globus-nothr-flavor=${GLOBUS_FLAVOR} --with-openssl-prefix=${EXTERNAL_STAGE} --with-voms-prefix=${EXTERNAL_STAGE} --with-boost-prefix=${CREAM_STAGE} --with-classads-prefix=${EXTERNAL_STAGE} --with-log4cpp-prefix=${CREAM_STAGE} --with-gsoap-location=${CREAM_STAGE} --with-glite-location=${CREAM_STAGE} --with-ce-wsdl-version=1.10.1 --with-delegation-wsdl-version=2.0.0
				    #--Build Step ----------
				    BUILD_COMMAND cd org.glite.ce.cream-client-api-c && make
				    BUILD_IN_SOURCE 1
				    #--install Step ----------
				    INSTALL_DIR ${EXTERNAL_STAGE}
				    INSTALL_COMMAND cd org.glite.ce.cream-client-api-c &&
						make install
				)

		# Set the target dependencies which the rest of condor depends on.
	set(CREAM_FOUND
	"${CREAM_STAGE}/lib/libglite_ce_cream_client_soap.a;${CREAM_STAGE}/lib/libglite_security_gsoap_plugin_${GLOBUS_FLAVOR}pthr.a;;${CREAM_STAGE}/lib/libglite_ce_cream_client_util.a;${CREAM_STAGE}/lib/liblog4cpp.a;${CREAM_STAGE}/lib/libgridsite.a;${CREAM_STAGE}/lib/libboost_thread-gcc-mt-d.a;${CREAM_STAGE}/lib/libares.a;${CREAM_STAGE}/lib/libboost_regex-gcc-mt-d.a")
	append_var(CONDOR_EXTERNALS cream)

	message (STATUS "external configured (CREAM_FOUND=${CREAM_FOUND})")
	set( CREAM_FOUND ${CREAM_FOUND} PARENT_SCOPE )
	set( HAVE_EXT_CREAM ON PARENT_SCOPE )
	set( CREAM_STAGE ${CREAM_STAGE} PARENT_SCOPE )

else()
	message (STATUS "external skiped (cream)")
endif()
