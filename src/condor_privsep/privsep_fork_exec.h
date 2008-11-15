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

#ifndef _PRIVSEP_FORK_EXEC
#define _PRIVSEP_FORK_EXEC

#include <stdio.h>

class ArgList;

class PrivSepForkExec {

public:
	PrivSepForkExec();
	~PrivSepForkExec();
	bool init();
	bool in_child(MyString&, ArgList&);
	FILE* parent_begin();
	bool parent_end();
private:
	FILE* m_in_fp;
	FILE* m_err_fp;
	int m_child_in_fd;
	int m_child_err_fd;
};

#endif