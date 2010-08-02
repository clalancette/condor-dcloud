
if ( NOT PROPER )

	option( WITH_HADOOP "Compiling with support for HADOOP" ON )

	if (WITH_HADOOP)

		set (HADOOP_VER hadoop-0.20.0)
		set ( HADOOP_INSTALL mkdir -p ${EXTERNAL_STAGE}/hdfs/conf ${CMD_TERM}
							 cp -r lib ${EXTERNAL_STAGE}/hdfs ${CMD_TERM}
							 cp ${HADOOP_VER}-p1-core.jar ${EXTERNAL_STAGE}/hdfs/lib ${CMD_TERM}
							 cp src/hdfs/hdfs-default.xml ${EXTERNAL_STAGE}/hdfs/conf )

		# a couple of odd things, this macro requires a configure and build command.  
		ExternalProject_Add(hadoop
							PREFIX ${EXTERNAL_BUILD_PREFIX}/${HADOOP_VER}
							#-- Download Step ----------
							DOWNLOAD_COMMAND wget -N http://parrot.cs.wisc.edu/externals/${HADOOP_VER}.tar.gz ${CMD_TERM}
							DOWNLOAD_DIR ${EXTERNAL_DL}
							URL http://parrot.cs.wisc.edu/externals/${HADOOP_VER}.tar.gz
							# PATCH_COMMAND patch -p0 < ${CMAKE_CURRENT_SOURCE_DIR}/hadoop-0.20.0-patch
							CONFIGURE_COMMAND echo "nothing to configure"
							BUILD_COMMAND echo "nothing to build"
							BUILD_IN_SOURCE 1
							#--install Step ----------
							INSTALL_DIR ${EXTERNAL_STAGE}
							INSTALL_COMMAND cd ${HADOOP_VER} ${CMD_TERM} ${HADOOP_INSTALL} )

		## Set all the config information
		set( HADOOP_TARGET ${HADOOP_TARGET} PARENT_SCOPE )
		set( HAVE_EXT_HADOOP ON PARENT_SCOPE )
		append_var(CONDOR_EXTERNALS hadoop)
		message (STATUS "external configured (hadoop)")
	else()
		message (STATUS "external skiped (hadoop)")
	endif()

else( NOT PROPER )

	message (STATUS "external skiped (hadoop)")

endif( NOT PROPER )

	



