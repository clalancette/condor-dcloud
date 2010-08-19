
condor_selective_glob("network2.*;server_interface.*" ServerAPISrcs )

condor_static_lib( ckpt_server_api "${ServerAPISrcs}" )
