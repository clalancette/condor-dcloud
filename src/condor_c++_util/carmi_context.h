#include <netinet/in.h>
#include "condor_types.h"
#include "manager.h"

enum {                  // how to get the information
        COLLECTOR,
        STARTD,
        SCHEDD,
        BOTH_DAEMONS
};

enum {          // types of values stored in the context
        CARMI_INT,
        CARMI_FLOAT,
        CARMI_STRING,
        CARMI_BOOL
};

class CARMI_Context {
public:
	CARMI_Context( const char *machine_name, int how = COLLECTOR, 
			int timeout = 0 );
#if 0
	CARMI_Context( unsigned int ip_addr, int how = COLLECTOR, 
				  int timeout = 0 );
	CARMI_Context( int how = COLLECTOR, int timeout = 0 );           // self 
#endif
    ~CARMI_Context();
	void display();

	int value( char *name, char *&val );
	int value( char *name, int &val );
	int value( char *name, float &val );
	int bool_value( char *name, int &val );

	void start_scan();
	int next( char *&name, int &type );

private:

MACH_REC		rec;
int				last_scan;
};
