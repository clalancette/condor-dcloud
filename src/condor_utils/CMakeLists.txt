
file( GLOB RmvSrcs y.tab* lex.yy* ckpt_server_api_stubs* libcondorapi_stubs* soap_helpers* test_* *.t.cpp *test.cpp cat_url.cpp *Test* )

# I've removed all the stubgen deps as I do not see
# (yet) how they are included into the utils project
# "stubgen" --?? Is this for standard univ?

condor_glob( HeaderFiles SourceFiles "${RmvSrcs}" )

# call perl to gen the params.  
command_target( utils_genparams perl param_info_c_generator.pl param_info_init.c )

condor_static_lib( utils "${HeaderFiles};${SourceFiles}" )

add_dependencies(utils utils_genparams syscall_numbers )
