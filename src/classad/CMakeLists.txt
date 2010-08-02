
# in order to update configure
#
if (NOT WINDOWS)
	ExternalProject_Add(classads
						PREFIX ${EXTERNAL_BUILD_PREFIX}/classads-devel
						#--Configure step ----------
						SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
						CONFIGURE_COMMAND
							#m4 Makefile.am.m4 > Makefile.am &&
							#autoreconf --install &&
							#rm -rf autom4te.cache &&
							./configure --without-pcre --disable-shared --enable-static --enable-namespace --prefix=${EXTERNAL_STAGE} CXXFLAGS=-DWANT_CLASSAD_NAMESPACE=1 CC=g++
						#--Build Step ----------
						BUILD_COMMAND make && make check
						BUILD_IN_SOURCE 1
						#--install Step ----------
						INSTALL_DIR ${EXTERNAL_STAGE}
						INSTALL_COMMAND make install)

	# Set the target dependencies which the rest of condor depends on.
	set(CLASSADS_FOUND "${EXTERNAL_STAGE}/lib/libclassad.a;${EXTERNAL_STAGE}/lib/libclassad_ns.a")	

else()
	
	add_definitions(-DCLASSAD_DISTRIBUTION)
	add_definitions(-DWANT_CLASSAD_NAMESPACE=1)
	
	# I should probably just add all and subtract tests.
	file( GLOB ClassadSrcs attrrefs.cpp common.cpp collection* fnCall.cpp expr* indexfile* lexer* literals.cpp matchClassad.cpp new_classad* query.cpp sink.cpp source.cpp transaction.cpp view.cpp xml* )	
	add_library( classads STATIC ${ClassadSrcs} )
	
	set( CLASSADS_FOUND classads )
	
endif()

message (STATUS "external configured (CLASSADS_FOUND=${CLASSADS_FOUND})")
set( CLASSADS_FOUND ${CLASSADS_FOUND} PARENT_SCOPE )
set( HAVE_EXT_CLASSADS ON PARENT_SCOPE )
append_var (CONDOR_EXTERNALS classads)

