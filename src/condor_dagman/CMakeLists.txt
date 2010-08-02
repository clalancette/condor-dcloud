
file( GLOB submitDAG condor_submit_dag* dagman_instantiate*)

condor_glob(DAGHdrs DAGSrcs "${submitDAG}")

condor_exe( condor_dagman "${DAGHdrs};${DAGSrcs}" ${C_BIN} "${CONDOR_LIBS};${PCRE_FOUND};${OPENSSL_FOUND};${KRB5_FOUND};${CLASSADS_FOUND};${COREDUMPER_FOUND}")

condor_exe(condor_submit_dag "condor_submit_dag.cpp;dagman_multi_dag.cpp;dagman_recursive_submit.cpp" ${C_BIN} "${CONDOR_TOOL_LIBS};${PCRE_FOUND};${OPENSSL_FOUND};${KRB5_FOUND};${CLASSADS_FOUND}")