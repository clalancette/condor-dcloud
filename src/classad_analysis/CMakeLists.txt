
condor_selective_glob( "analysis.*;boolExpr.*;condition.*;multiProfile.*;profile.*;resourceGroup.*;boolValue.*;explain.*;interval.*;result.*" AnalSrcs )

condor_static_lib( analysis "${AnalSrcs}" )
condor_static_lib( conversion "conversion.cpp;conversion.h" )
# # condor_static_lib( gangmatch "portGraph.cpp;gmr.cpp;gangster.cpp")