

if (HAVE_EXT_GSOAP AND HAVE_EXT_OPENSSL AND HAVE_PTHREADS)

	if (PRE_WSEAPPI_CPP)
	  add_definitions(-DPRE_WSEAPPI_CPP)
	endif(PRE_WSEAPPI_CPP)
	
	set (AMAZON_SOAP_FILES "soapC.cpp;soapClient.cpp;soapH.h;soapStub.h;soapAmazonEC2Object.h;soapAmazonEC2Proxy.h")

	file( GLOB RmvSrcs *ec2* ${AMAZON_SOAP_FILES} soapServer.cpp wsseapi.cpp smdevp.cpp )
	condor_glob( HeaderFiles SourceFiles "${RmvSrcs}" )

	include_directories(${GSOAP_SHARE}/plugin/)
	include_directories(${CONDOR_SOURCE_DIR}/src/condor_amazon)

	command_target(gen_amazon_soapfiles ${SOAPCPP2} "-L;-w;-x;-I${GSOAP_SHARE}/import;gsoap_ec2.h" "${AMAZON_SOAP_FILES};soapServer.cpp;AmazonEC2.nsmap")

	condor_exe( amazon_gahp
	"${HeaderFiles};${SourceFiles};${AMAZON_SOAP_FILES};${WSSEAPI};${SMDEVP}" ${C_SBIN}
	"${CONDOR_TOOL_LIBS};${PCRE_FOUND};${OPENSSL_FOUND};${KRB5_FOUND};${GSOAP_FOUND};${ZLIB_FOUND}")

	add_dependencies(gen_amazon_soapfiles gsoap)
	set_source_files_properties (${WSSEAPI} ${SMDEVP} PROPERTIES GENERATED TRUE)

	condor_exe_test(ec2_sim "ec2_sim_main.cpp;ec2_sim_impl.cpp;soapC.cpp;soapServer.cpp" "${CONDOR_TOOL_LIBS};${PCRE_FOUND};${OPENSSL_FOUND};${KRB5_FOUND};${GSOAP_FOUND};${ZLIB_FOUND}")

endif()
