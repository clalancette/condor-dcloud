
file( GLOB SysApiRmvElements *_t.cpp *.t.* dhry21b* )

condor_glob(SysapiHeaderFiles SysapiSourceFiles "${SysApiRmvElements}" )

#create the library
condor_static_lib(sysapi "${SysapiHeaderFiles};${SysapiSourceFiles}")

# there was a test target which was never used.
# it makes the most sense to hook in a UT here instead of integ test