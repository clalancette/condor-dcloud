
file( GLOB DCRmvElements *.t.* )

if (NOT HAVE_EXT_CLASSADS)	
	file( GLOB DCRmvElements dc_credd* dc_lease_manager* ${DCRmvElements})	
endif()

condor_glob(DCHeaderFiles DCSourceFiles "${DCRmvElements}")

#create the library
condor_static_lib(daemon_client "${DCHeaderFiles};${DCSourceFiles}" )
