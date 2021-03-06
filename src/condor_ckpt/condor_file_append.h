/***************************************************************
 *
 * Copyright (C) 1990-2007, Condor Team, Computer Sciences Department,
 * University of Wisconsin-Madison, WI.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License"); you
 * may not use this file except in compliance with the License.  You may
 * obtain a copy of the License at
 * 
 *    http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************/


#ifndef CONDOR_FILE_APPEND_H
#define CONDOR_FILE_APPEND_H

/*
This class passes all operations through, except open.
For open(), it forces the O_APPEND flag to be set.
*/


#include "condor_file.h"

class CondorFileAppend : public CondorFile {
public:
	CondorFileAppend( CondorFile *original );
	virtual ~CondorFileAppend();

	virtual int open( const char *url, int flags, int mode );
	virtual int close();
	virtual int read( int offset, char *data, int length );
	virtual int write( int offset, char *data, int length );

	virtual int fcntl( int cmd, int arg );
	virtual int ioctl( int cmd, long arg );
	virtual int ftruncate( size_t length ); 
	virtual int fsync();
	virtual int flush();
	virtual int fstat( struct stat *buf );

	virtual int	is_readable();
	virtual int	is_writeable();
	virtual int	is_seekable();

	virtual int	get_size();
	virtual char const	*get_url();

	virtual int get_unmapped_fd();
	virtual int is_file_local();

private:
	char *url;
	CondorFile *original;
};

#endif
