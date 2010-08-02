
if (WANT_OLD_CLASSADS)
	file( GLOB ClassAdsOldRmvSrcs classad_lookup* *test*)

	condor_glob( ClassAdsOldHdrs ClassAdsOldSrcs "${ClassAdsOldRmvSrcs}" )

	#create the library
	condor_static_lib( classad.old "${ClassAdsOldHdrs};${ClassAdsOldSrcs}" )
else()
	message(STATUS "NOTE: skipping classads.old")
endif()