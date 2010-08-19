condor_glob(ccbHdrs ccbSrcs "")
condor_static_lib( ccb "${ccbHdrs};${ccbSrcs}" )

