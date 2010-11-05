
if (NOT WINDOWS)

	install ( FILES condor_glidein
				condor_run
			DESTINATION ${C_BIN} )

	install ( FILES condor_configure
				condor_updates_stats
				condor_init
			DESTINATION ${C_SBIN} )

	install ( FILES condor_configure DESTINATION ${C_SBIN} RENAME condor_install )
	install (CODE "EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E create_symlink \${CMAKE_INSTALL_PREFIX}/${C_SBIN}/condor_install \${CMAKE_INSTALL_PREFIX}/condor_install)")
	install (CODE "EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E create_symlink \${CMAKE_INSTALL_PREFIX}/${C_SBIN}/condor_configure \${CMAKE_INSTALL_PREFIX}/condor_configure)")
	

	install ( FILES condor_ssh
				sshd.sh
			DESTINATION ${C_LIBEXEC} )

	install ( FILES condorc-advertiser
				condorc-authorizer
				condorc-initialize
				condorc-launcher
				condorc-vo-advertiser
				grid_monitor.sh
			DESTINATION ${C_LIBEXEC}/glite )

	install ( FILES Condor.pm DESTINATION ${C_LIB} )

	if (STD_UNIVERSE)
		install ( FILES condor_compile DESTINATION ${C_BIN} )
		install ( FILES ld DESTINATION ${C_LIB} )
		install ( FILES ld DESTINATION ${C_LIB} RENAME real-ld )
	endif(STD_UNIVERSE)

endif (NOT WINDOWS)