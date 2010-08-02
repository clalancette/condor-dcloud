
file( GLOB ServerAPISrcs network2.* server_interface.* )
#condor_glob( CedarHdrs CedarSrcs "${CeadarRmvSrcs}" )

#create the library
condor_static_lib( ckpt_server_api "${ServerAPISrcs}" )
