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

#ifndef _PRIVSEP_HELPER_H
#define _PRIVSEP_HELPER_H

class ArgList;
class Env;
struct FamilyInfo;

class PrivSepHelper {

public:

	// change ownership of the sandbox to the user
	//
	virtual void chown_sandbox_to_user() = 0;

	// change ownership of the sandbox to condor
	//
	virtual void chown_sandbox_to_condor() = 0;

	// change our state to "sandbox is owned by user"
	virtual void set_sandbox_owned_by_user() = 0;

	// launch the job as the user
	//
	virtual int create_process(const char* path,
	                           ArgList&    args,
	                           Env&        env,
	                           const char* iwd,
	                           int         std_fds[3],
	                           const char* std_file_names[3],
	                           int         nice_inc,
	                           size_t*     core_size_ptr,
	                           int         reaper_id,
	                           int         dc_job_opts,
	                           FamilyInfo* family_info,
							   int *       affinity_mask = 0) = 0;

	virtual ~PrivSepHelper() { }
};

#endif
