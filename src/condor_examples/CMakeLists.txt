if (NOT WINDOWS)

install (FILES
		condor.boot.generic
		condor_config.local.central.manager
		condor_config.local.ha_condor_client
		condor.sysconfig
		README
		condor.boot.rpm
		condor_config.local.ckpt.server
		condor_config.local.ha_condor_had
		lamscript
		condor_config.generic
		condor_config.local.dedicated.resource
		condor_config.local.view_server
		mp1script
		condor_config.generic.debian.patch
		condor_config.local.dedicated.submit
		condor_config.submit.generic
		mp2script
		condor_config.generic.rpm.patch
		condor_config.local.generic
		condor.init
		openmpiscript
		DESTINATION ${C_ETC})

install (FILES condor_limits_wrapper.sh DESTINATION ${C_LIBEXEC})


endif(NOT WINDOWS)