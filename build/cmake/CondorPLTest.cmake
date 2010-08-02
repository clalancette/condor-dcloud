MACRO ( CONDOR_PL_TEST _TARGET _DESC _TEST_RUNS )

	foreach(test ${_TEST_RUNS})
		file (APPEND list_${test} "${_TARGET}\n")
	endforeach(test)
	
	# add to all targets 
	file (APPEND list_all "${_TARGET}\n")

	# I'm not certain but it appears that the description files are not gen'd
	# file ( APPEND ${_TARGET}.desc ${_DESC} )

ENDMACRO ( CONDOR_PL_TEST )