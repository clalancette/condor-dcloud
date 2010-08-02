
file( GLOB starterRmvElements condor_rsh.cpp glexec_job_wrapper* )

condor_daemon( starter "${starterRmvElements}" "${CONDOR_LIBS};${PCRE_FOUND};${OPENSSL_FOUND};${COREDUMPER_FOUND};${KRB5_FOUND};qmgmt" "${C_SBIN}" OFF)
clone_install( condor_starter "condor_gridshell" "${C_SBIN}" )

# need to add all the other install targets for libexec.
install ( FILES scimark2lib.jar CondorJavaWrapper.class CondorJavaInfo.class DESTINATION ${C_LIB} )

if(${OS_NAME} MATCHES "LINUX")
	install (FILES condor_glexec_setup condor_glexec_run condor_glexec_cleanup condor_glexec_kill condor_glexec_update_proxy DESTINATION ${C_LIBEXEC})
	condor_exe(condor_glexec_job_wrapper "glexec_job_wrapper.linux.cpp" ${C_LIBEXEC} "${CONDOR_TOOL_LIBS};${PCRE_FOUND}" )
endif()

if (HAVE_SSH_TO_JOB)
	install ( FILES condor_ssh_to_job_sshd_setup condor_ssh_to_job_shell_setup DESTINATION ${C_LIBEXEC} )
	install ( FILES condor_ssh_to_job_sshd_config_template DESTINATION ${C_LIB} )
endif(HAVE_SSH_TO_JOB)
