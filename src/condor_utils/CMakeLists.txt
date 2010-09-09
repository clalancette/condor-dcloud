
file( GLOB RmvSrcs y.tab* lex.yy* ckpt_server_api_stubs* libcondorapi_stubs* soap_helpers* test_* *.t.cpp *test.cpp cat_url.cpp *Test* )

condor_glob( HeaderFiles SourceFiles "${RmvSrcs}" )
command_target( utils_genparams perl param_info_c_generator.pl param_info_init.c )
condor_static_lib( utils "${HeaderFiles};${SourceFiles}" )
add_dependencies(utils utils_genparams syscall_numbers )

condor_selective_glob("my_username.*;condor_event.*;file_sql.*;misc_utils.*;user_log_header.*;write_user_log*;read_user_log*;iso_dates.*;file_lock.*;format_time.*;utc_time.*;stat_wrapper*;log_rotate.*;dprintf*;sig_install.*;basename.*;mkargv.*;except.*;strupr.*;lock_file.*;rotate_file.*;strcasestr.*;strnewp.*;condor_environ.*;setsyscalls.*;passwd_cache.*;uids.*;chomp.*;subsystem_info.*;my_subsystem.*;distribution.*;my_distribution.*;get_random_num.*;libcondorapi_stubs.*;seteuid.*;setegid.*;condor_open.*;classad_merge.*;condor_attributes.*;simple_arg.*;compat_classad.*;compat_classad_util.*;classad_oldnew.*;condor_snutils.*;stringSpace.*;string_list.*;MyString.*;condor_xml_classads.*" ApiSrcs)
condor_static_lib(condorapi "${ApiSrcs}")

if (NOT WINDOWS)
	set(APILIBDEP dl)
else()
	#set(APILIBDEP dl)
endif()

target_link_libraries(condorapi ${CLASSADS_FOUND};${APILIBDEP})

add_definitions(-DENABLE_STATE_DUMP)
condor_exe_test(test_log_reader "test_log_reader.cpp" "condorapi")
condor_exe_test(test_log_reader_state "test_log_reader_state.cpp" "condorapi")
condor_exe_test(test_log_writer "test_log_writer.cpp" "condorapi")
condor_exe_test(test_libcondorapi "test_libcondorapi.cpp" "condorapi")

