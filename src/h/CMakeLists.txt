#set the local target 
set(TargetName syscall_numbers)

add_custom_command( OUTPUT syscall_numbers.h 
					COMMAND awk
					ARGS -f awk_prog.include_file syscall_numbers.tmpl > syscall_numbers.h )
					
add_custom_command( OUTPUT syscall_numbers.cpp 
					COMMAND awk
					ARGS -f awk_prog.name_table syscall_numbers.tmpl > syscall_numbers.cpp )
					
condor_static_lib( syscall_numbers "syscall_numbers.cpp;syscall_numbers.h" )



