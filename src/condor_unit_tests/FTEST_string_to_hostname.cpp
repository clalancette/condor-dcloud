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

/*
	This code tests the sin_to_string() function implementation.
 */

#include "condor_common.h"
#include "condor_debug.h"
#include "condor_config.h"
#include "internet.h"
#include "function_test_driver.h"
#include "unit_test_utils.h"
#include "emit.h"

static bool test_normal_case(void);

bool FTEST_string_to_hostname(void) {
		// beginning junk for getPortFromAddr(() {
	emit_function("string_to_hostname(const char*)");
	emit_comment("Converts a sinful string to a hostname.");
	emit_problem("None");
	
		// driver to run the tests and all required setup
	FunctionDriver driver;
	driver.register_function(test_normal_case);
	
		// run the tests
	return driver.do_all_functions();
}

static bool test_normal_case() {
	emit_test("Is normal input converted correctly?");
	char *who = "cs.wisc.edu";
	char buf[1024];
	char instring[35];
	char* input = &instring[0];
	char* host_to_test = strdup( who );
	struct hostent *h;
	h = gethostbyname(host_to_test);
	if (h == NULL) {
		sprintf(buf, "A reverse lookup for '%s' which *must* succeed for this "
				"unit test to function has failed! Test FAILED.", who);
		emit_alert(buf);
		FAIL;
	}
	free(host_to_test);
	emit_input_header();
	sprintf(instring, "<%s:100>",inet_ntoa(*((in_addr*) h->h_addr)));
	emit_param("IP", instring);
	emit_output_expected_header();
	char expected[30];
	sprintf(expected, who);
	emit_retval(expected);
	emit_output_actual_header();
	char* hostname = string_to_hostname(input);
	emit_retval(hostname);
	if(strcmp(&expected[0], hostname) != 0) {
		FAIL;
	}
	PASS;
}
