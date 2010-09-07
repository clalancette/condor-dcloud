
condor_selective_glob("network2.*;server_interface.*" ServerAPISrcs )

condor_static_lib( ckpt_server_api "${ServerAPISrcs}" )

dprint("TSTCLAIR: TODO")

# OBJ = network2.o gen_lib.o fileinfo2.o fileindex2.o imds2.o xferstat2.o \
#	signal2.o alarm2.o server2.o xfer_summary.o replication.o protocol.o

# public_c_plus_target($(NAME),sbin,$(OBJ),$(LIB))