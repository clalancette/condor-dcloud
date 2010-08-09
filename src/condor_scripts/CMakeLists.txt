
if (NOT WINDOWS)

	install ( FILES condor_glidein
				condor_run
			DESTINATION ${C_BIN} )

	install ( FILES condor_configure
				condor_updates_stats
				condor_init
			DESTINATION ${C_SBIN} )

	install ( FILES condor_configure DESTINATION ${C_SBIN} RENAME condor_install )

	if (NOT CLIPPED)
		install ( FILES condor_compile DESTINATION ${C_BIN} )
		install ( FILES ld DESTINATION ${C_LIB} )
		install ( FILES ld DESTINATION ${C_LIB} RENAME real-ld )
	endif(NOT CLIPPED)

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

endif (NOT WINDOWS)