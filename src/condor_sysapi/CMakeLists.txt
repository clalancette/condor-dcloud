
file( GLOB SysApiRmvElements *_t.cpp *.t.* dhry21b* )

condor_glob(SysapiHeaderFiles SysapiSourceFiles "${SysApiRmvElements}" )

#create the library
condor_static_lib(sysapi "${SysapiHeaderFiles};${SysapiSourceFiles}")