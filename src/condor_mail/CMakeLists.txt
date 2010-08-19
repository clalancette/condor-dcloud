if(WINDOWS)

	# In 2.8 there are issues with including .NET projects, so sadly I have decided to prebuild :-( 
	# path exists in their repo and 2.8.1 we can fix. 
	# include_external_msproject(condor_mail ${CONDOR_SOURCE_DIR}/src/tools/mail/condor_mail.vcproj )

endif(WINDOWS)