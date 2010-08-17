

if (HAVE_EXT_GSOAP AND HAVE_EXT_OPENSSL AND HAVE_PTHREADS)

	
	set (AMAZON_SOAP_FILES "soapC.cpp;soapClient.cpp;soapH.h;soapStub.h;soapAmazonEC2Object.h;soapAmazonEC2Proxy.h;")

	file( GLOB RmvSrcs *ec2* ${AMAZON_SOAP_FILES} soapServer.cpp wsseapi.cpp smdevp.cpp )
	condor_glob( HeaderFiles SourceFiles "${RmvSrcs}" )

	include_directories(${GSOAP_SHARE}/plugin/)
	include_directories(${CONDOR_SOURCE_DIR}/src/condor_amazon)

	command_target(gen_amazon_soapfiles ${SOAPCPP2} "-L;-w;-x;-I${GSOAP_SHARE}/import;gsoap_ec2.h" "${AMAZON_SOAP_FILES};soapServer.cpp;AmazonEC2.nsmap")
	command_target(cp_wsseapi cp "${GSOAP_SHARE}/plugin/wsseapi.c;wsseapi.cpp" "wsseapi.cpp")
	command_target(cp_smdevp cp "${GSOAP_SHARE}/plugin/smdevp.c;smdevp.cpp" "smdevp.cpp")
	command_target(patch_wsseapi patch "-p3;<;wsseapi.patch" "wsseapi.cpp.orig")

	list(APPEND AMAZON_SOAP_FILES wsseapi.cpp smdevp.cpp )

	condor_exe( amazon_gahp
	"${HeaderFiles};${SourceFiles};${AMAZON_SOAP_FILES}" ${C_SBIN}
	"${CONDOR_TOOL_LIBS};${PCRE_FOUND};${OPENSSL_FOUND};${KRB5_FOUND};${GSOAP_FOUND};${ZLIB_FOUND}")

	add_dependencies(gen_amazon_soapfiles gsoap)
	add_dependencies(cp_wsseapi gsoap)
	add_dependencies(cp_smdevp gsoap)
	add_dependencies(patch_wsseapi gsoap)
	
	add_dependencies(patch_wsseapi cp_wsseapi)
	add_dependencies(amazon_gahp gen_amazon_soapfiles cp_wsseapi patch_wsseapi cp_smdevp)

	condor_exe_test(ec2_sim "ec2_sim_main.cpp;ec2_sim_impl.cpp;soapC.cpp;soapServer.cpp" "${CONDOR_TOOL_LIBS};${PCRE_FOUND};${OPENSSL_FOUND};${KRB5_FOUND};${GSOAP_FOUND};${ZLIB_FOUND}")

endif()
