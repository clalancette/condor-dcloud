
file( GLOB DCRmvElements dc_credd* dc_lease_manager* *.t* )
condor_glob(DCHeaderFiles DCSourceFiles "${DCRmvElements}")

#create the library
condor_static_lib(daemon_client "${DCHeaderFiles};${DCSourceFiles}" )
