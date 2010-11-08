option( WITH_BOOST "Compiling with support for BOOST" ON )

if (WITH_BOOST)

  if (NOT PROPER)

		if (WINDOWS)
		  # need to expand the configure and 
		  set(TOOLSET msvc)
		  set(JAM_EXT bat)
		else()
		  set(TOOLSET gcc)
		  set(JAM_EXT sh)
		endif()
  
		# so the idea of privately staging boost is silly.  
		set (BOOST_VER 1_33_1)
		ExternalProject_Add(boost
				    PREFIX ${EXTERNAL_BUILD_PREFIX}/boost_${BOOST_VER}
				    #-- Download Step ----------
					DOWNLOAD_COMMAND wget -N http://parrot.cs.wisc.edu/externals/boost_${BOOST_VER}.tar.gz ${CMD_TERM}
				    DOWNLOAD_DIR ${EXTERNAL_DL}
				    URL http://parrot.cs.wisc.edu/externals/boost_${BOOST_VER}.tar.gz
				    #--Configure step ----------
				    CONFIGURE_COMMAND cd boost_${BOOST_VER}/tools/build/jam_src ${CMD_TERM}
				    	./build.${JAM_EXT} ${CMD_TERM}
						mv `find . | grep bjam` ../../../
				    #--Build Step ----------
				    BUILD_COMMAND cd boost_${BOOST_VER} && bjam -sNO_COMPRESSION=1 -sTOOLS=${TOOLSET} --prefix=${EXTERNAL_STAGE} link=static install
				    BUILD_IN_SOURCE 1
				    #--install Step ----------
				    INSTALL_DIR ${EXTERNAL_STAGE}
				    INSTALL_COMMAND cd ${EXTERNAL_STAGE}/include && mv -f boost-1_33_1/boost . )

	set (BOOST_INCLUDE_FOUND ${EXTERNAL_STAGE}/include)
	append_var(CONDOR_EXTERNALS boost)
	
  else(NOT PROPER)

	# For proper builds use the .so/.dll's .
	set(Boost_USE_MULTITHREADED ON)
	find_package( Boost 1.33.1 )
	if(Boost_FOUND)
		set (BOOST_INCLUDE_FOUND ${Boost_INCLUDE_DIRS})
		set (BOOST_VER ${Boost_VERSION})
	endif(Boost_FOUND)

  endif(NOT PROPER) 

endif(WITH_BOOST)

# update configure information
if (BOOST_VER)
	message (STATUS "external configured (BOOST_INCLUDE_FOUND=${BOOST_INCLUDE_FOUND}) version:(${BOOST_VER})")
	set( HAVE_EXT_BOOST ON PARENT_SCOPE )
	set (BOOST_INCLUDE_FOUND ${BOOST_INCLUDE_FOUND} PARENT_SCOPE)
else(BOOST_VER)
	message (STATUS "external skiped (boost)")
endif(BOOST_VER)