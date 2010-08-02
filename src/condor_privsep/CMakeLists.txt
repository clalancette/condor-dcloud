
file( GLOB PrivsepRmvSrcs switchboard* parse_config* safe* )

condor_glob(PrivsepHdrs PrivsepSrcs "${PrivsepRmvSrcs}")

condor_static_lib( privsep "${PrivsepHdrs};${PrivsepSrcs}" )

condor_exe( condor_root_switchboard "${PrivsepRmvSrcs}" ${C_SBIN} "${CONDOR_TOOL_LIBS}" )