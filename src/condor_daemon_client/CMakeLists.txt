
file( GLOB DCRmvElements *.t* )
condor_glob(DCHeaderFiles DCSourceFiles "${DCRmvElements}")

#create the library
condor_static_lib(daemon_client "${DCHeaderFiles};${DCSourceFiles}" )
