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

//******************************************************************************
//
// scanner.h
//
//******************************************************************************

#ifndef _SCANNER_H_
#define _SCANNER_H_

#if !defined(WANT_OLD_CLASSADS)

#include "compat_classad.h"
#include "compat_classad_list.h"
#include "compat_classad_util.h"
using namespace compat_classad;

#else


#define MAXVARNAME 256

#define USE_NEW_SCANNER

#include "condor_attrlist.h"

class Token
{
	public:

		Token();
		~Token();
		void		reset();
		union
		{
			int		intVal;
			float	floatVal;
		};
		LexemeType	type; 
		int			length;	// error position in the string for the parser
#ifdef USE_NEW_SCANNER
		char        *strVal;
		int         strValLength;
#else
		char		strVal[ATTRLIST_MAX_EXPRESSION];
#endif
		friend	void	Scanner(const char *&input, Token &token);

	private:

		int		isString;

};

extern	void		Scanner(const char *&, Token&);


#endif /* !defined(WANT_OLD_CLASSADS) */

#endif /* _SCANNER_H */
