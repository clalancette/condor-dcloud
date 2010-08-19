

condor_selective_glob( "procapi.h;procapi.cpp;processid.*;procapi_killfamily.cpp" ProcAPISrcs)
condor_static_lib ( procapi "${ProcAPISrcs}" )

# there was a test target which was never used.
# it makes the most sense to hook in a UT here instead of integ test
