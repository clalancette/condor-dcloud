
#file( GLOB ccbRmvSrcs  )
condor_glob(ccbHdrs ccbSrcs "")

#create the library
condor_static_lib( ccb "${ccbHdrs};${ccbSrcs}" )

