

file( GLOB ProcAPISrcs procapi.cpp processid.cpp procapi_killfamily.cpp )

# there are a series of other targets as well
# including a test binary and test exe's...
# procinterface being one of them.

#create the library
condor_static_lib ( procapi "${ProcAPISrcs}" )

# there was a test target which was never used.
# it makes the most sense to hook in a UT here instead of integ test
