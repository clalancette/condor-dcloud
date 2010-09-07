MACRO (GLIBC_DETECT _VERSION)

# there are multiple ways to detect glibc, but given nmi's
# cons'd up paths I will trust only gcc.  I guess I could also use
# ldd --version to detect.

	set(_GLIB_SOURCE_DETECT "
#include <limits.h>
#include <stdio.h>
int main()
{
  printf(\"%d%d\",__GLIBC__, __GLIBC_MINOR__);
  return 0;
}
")

file (WRITE ${CONDOR_SOURCE_DIR}/build/cmake/glibc.cpp "${_GLIB_SOURCE_DETECT}\n")

try_run(POST26_GLIBC_DETECTED
		POST26_GLIBC_COMPILE
        ${CONDOR_SOURCE_DIR}/build/cmake
		${CONDOR_SOURCE_DIR}/build/cmake/glibc.cpp
        RUN_OUTPUT_VARIABLE GLIBC_VERSION )

if (GLIBC_VERSION AND POST26_GLIBC_COMPILE )
	dprint("GLIBC_VERSION=${GLIBC_VERSION}")
	set(${_VERSION} ${GLIBC_VERSION})
else()
	message(STATUS "NOTE: Could not detect GLIBC_VERSION from copiler")
endif()


ENDMACRO (GLIBC_DETECT)