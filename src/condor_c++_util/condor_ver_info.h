/***************************Copyright-DO-NOT-REMOVE-THIS-LINE**
 * CONDOR Copyright Notice
 *
 * See LICENSE.TXT for additional notices and disclaimers.
 *
 * Copyright (c)1990-1998 CONDOR Team, Computer Sciences Department, 
 * University of Wisconsin-Madison, Madison, WI.  All Rights Reserved.  
 * No use of the CONDOR Software Program Source Code is authorized 
 * without the express consent of the CONDOR Team.  For more information 
 * contact: CONDOR Team, Attention: Professor Miron Livny, 
 * 7367 Computer Sciences, 1210 W. Dayton St., Madison, WI 53706-1685, 
 * (608) 262-0856 or miron@cs.wisc.edu.
 *
 * U.S. Government Rights Restrictions: Use, duplication, or disclosure 
 * by the U.S. Government is subject to restrictions as set forth in 
 * subparagraph (c)(1)(ii) of The Rights in Technical Data and Computer 
 * Software clause at DFARS 252.227-7013 or subparagraphs (c)(1) and 
 * (2) of Commercial Computer Software-Restricted Rights at 48 CFR 
 * 52.227-19, as applicable, CONDOR Team, Attention: Professor Miron 
 * Livny, 7367 Computer Sciences, 1210 W. Dayton St., Madison, 
 * WI 53706-1685, (608) 262-0856 or miron@cs.wisc.edu.
****************************Copyright-DO-NOT-REMOVE-THIS-LINE**/
#ifndef CONDOR_VER_INFO_H
#define CONDOR_VER_INFO_H


/** Class to interpret the Condor Version string. 
Every Condor binary contains a version string embedded into it at
compile time.  This class extracts that version string and can parse it.
Furthermore, this class can be used to determine if different components
of Condor are compatible with one another.	*/
class CondorVersionInfo
{
public:

	/** Constructor.  Pass in the version string and subsystem to parse.
		Typically these parameters are NULL, then the version string and 
		subsystem compiled into this binary are used.  Other common sources 
		for a versionstring could be from the get_version_from_file() 
		method (which is static, so it can be invoked before calling
		the constructor), or passed as part of a protocol, etc.
		@param versionstring See condor_version.C for format.
		@param subssytem One of SHADOW, STARTER, TOOL, SCHEDD, COLLECTOR, etc.
	*/
	CondorVersionInfo::CondorVersionInfo(char *versionstring = NULL, 
		char *subsystem = NULL);

	/// Destructor.
	CondorVersionInfo::~CondorVersionInfo();

	static char *get_version_from_file(const char* filename, 
							char *ver = NULL, int maxlen = 0);

	/** Return the first number in the version ID.
		@return -1 on error */
	int getMajorVer() 
		{ return myversion.MajorVer > 5 ? myversion.MajorVer : -1; }
	/** Return the second number in the version ID (the series id).
		@return -1 on error */
	int getMinorVer() 
		{ return myversion.MajorVer > 5 ? myversion.MinorVer : -1; }
	/** Return the third number in the version ID (release id within the series).
		@return -1 on error */
	int getSubMinorVer() 
		{ return myversion.MajorVer > 5 ? myversion.SubMinorVer : -1; }
	
	/** Report if this version id represents a stable or developer series 
		release.
		@return true if a stable series, otherwise false. */
	bool is_stable_series() 
		{ return (myversion.MinorVer % 2 == 0);  }


	int compare_versions(const char* other_version_string);
	int compare_build_dates(const char* other_version_string);
	
	bool built_since_version(int MajorVer, int MinorVer, int SubMinorVer);
	bool built_since_date(int month, int day, int year);

	bool is_compatible(const char* other_version_string, 
								 const char* other_subsys = NULL);

	bool is_valid(const char* VersionString = NULL);

	typedef struct VersionData {
		int MajorVer;
		int MinorVer;
		int SubMinorVer;
		int Scalar;
		time_t BuildDate;
	} VersionData_t;


private:
	
	VersionData_t myversion;
	char *mysubsys;

	bool string_to_VersionData(const char *,VersionData_t &);
};



#endif