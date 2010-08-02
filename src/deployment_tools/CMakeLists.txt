
if (NOT WINDOWS)

	install ( FILES cleanup_release
				condor_cleanup_local
				condor_cold_start
				condor_cold_stop
				condor_config_bind
				filelock_midwife
				filelock_undertaker
				condor_install_local
				condor_local_start
				condor_local_stop
				install_release
			DESTINATION ${C_SBIN} )

	install ( FILES Execute.pm FileLock.pm DESTINATION ${C_LIB} )

	condor_exe( uniq_pid_command "uniq_pid_tool_main.cpp" ${C_SBIN} "${CONDOR_TOOL_LIBS};${PCRE_FOUND}" )
	clone_install( uniq_pid_command uniq_pid_midwife ${C_SBIN} )
	clone_install( uniq_pid_command uniq_pid_undertaker ${C_SBIN} )

endif (NOT WINDOWS)