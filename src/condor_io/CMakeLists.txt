
file( GLOB CeadarRmvSrcs *.t* test.h )
condor_glob( CedarHdrs CedarSrcs "${CeadarRmvSrcs}" )

#create the library
condor_static_lib(cedar "${CedarHdrs};${CedarSrcs}")

