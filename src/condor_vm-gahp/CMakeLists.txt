if(NOT WIN_EXEC_NODE_ONLY)

	file( GLOB RmvSrcs *_t.cpp )
	condor_glob( HeaderFiles SourceFiles "${RmvSrcs}" )

	condor_exe( condor_vm-gahp "${HeaderFiles};${SourceFiles}" ${C_SBIN}
	"${CONDOR_LIBS};${PCRE_FOUND};${OPENSSL_FOUND};${KRB5_FOUND};${LIBVIRT_FOUND};${LIBXML2_FOUND};${COREDUMPER_FOUND};${ZLIB_FOUND}")

	install ( FILES condor_vm_xen.sh condor_vm_vmware.pl DESTINATION ${C_SBIN} )
	install ( FILES libvirt_simple_script.awk DESTINATION ${C_LIBEXEC} )
	
endif(NOT WIN_EXEC_NODE_ONLY)
