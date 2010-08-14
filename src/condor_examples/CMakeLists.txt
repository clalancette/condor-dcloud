if (NOT WINDOWS)

install(FILES		condor.sysconfig
		DESTINATION	"${C_SYSCONFIG}"
		PERMISSIONS	OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
		RENAME		condor)

install(FILES		condor_config.local.generic
		DESTINATION	"${C_ETC}"
		RENAME		condor_config.local)

install (FILES
		condor.boot.generic
		condor_config.local.central.manager
		condor_config.local.ha_condor_client
		#condor.sysconfig
		README
		#condor.boot.rpm
		condor_config.local.ckpt.server
		condor_config.local.ha_condor_had
		lamscript
		condor_config.generic
		condor_config.local.dedicated.resource
		condor_config.local.view_server
		mp1script
		#condor_config.generic.debian.patch
		condor_config.local.dedicated.submit
		condor_config.submit.generic
		mp2script
		#condor_config.generic.rpm.patch
		#condor_config.local.generic
		condor.init
		openmpiscript
		DESTINATION ${C_ETC_EXAMPLES})

install (FILES condor_limits_wrapper.sh DESTINATION ${C_LIBEXEC})


#Prepare OS specific config file

if (( ${LINUX_NAME} STREQUAL "Redhat" ) OR ( ${LINUX_NAME} STREQUAL "Fedora" ) )

	add_custom_command( OUTPUT condor_config.patched
						WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
						COMMAND patch 
						ARGS condor_config.generic condor_config.generic.rpm.patch -o condor_config.patched )

	add_custom_target(	patch_configurations
						ALL
						DEPENDS condor_config.patched)

	install(FILES		condor_config.patched
			DESTINATION	"${C_ETC}"
			RENAME	condor_config)

	install(FILES		condor.boot.rpm
			DESTINATION	"${C_INIT}"
			PERMISSIONS	OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
			RENAME		condor)

elseif ( ${LINUX_NAME} STREQUAL "Debian" )

	add_custom_command( OUTPUT condor_config.patched
						WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
						COMMAND patch 
						ARGS condor_config.generic condor_config.generic.debian.patch -o condor_config.patched )

	add_custom_target(	patch_configurations
						ALL
						DEPENDS condor_config.patched)

	install(FILES		condor_config.patched
						DESTINATION	"${C_ETC}"
						RENAME	condor_config)

	#Debian init script need to be patched because /etc/sysconfig/condor -> /etc/default/condor
	add_custom_command( OUTPUT condor.boot.debian
						WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
						COMMAND patch 
						ARGS condor.boot.rpm condor.boot.debian.patch -o condor.boot.debian )

	add_custom_target(	patch_init_script
						ALL
						DEPENDS condor.boot.debian)

	install(FILES		condor.boot.debian
			DESTINATION	"${C_INIT}"
			PERMISSIONS	OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
			RENAME	condor)
else()
	#Other Platform
	install(FILES		condor_config.generic
			DESTINATION	"${C_ETC}"
			RENAME	condor_config)

	install(FILES		condor.boot.rpm
			DESTINATION	"${C_INIT}"
			PERMISSIONS	OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
			RENAME		condor)

endif()


endif(NOT WINDOWS)