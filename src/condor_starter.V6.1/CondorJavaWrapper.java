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

/*
XXX XXX XXX
WARNING WARNING WARNING
If you change this file, then you must compile it
and check the .class file into CVS.  Why?  Because
we can't manage a Java installation on all of our platforms,
but we still want compiled Java code distributed with
all platforms.
*/

/* 
This class executes a subprogram and produces a set of files
that carefully describe what happened.

Use: CondorJavaWrapper <startfile> <endfile> <program> <args>

Before the program starts, <startfile> is simply created and empty.
After the program finishes, <endfile> is created and contains
a description of how the program completed.  By examining these two
files, we may break down exactly what happened to the program.

The first line of <endfile> is either "normal", "abnormal", or
"noexec".

If "normal", then the program fell off the end of main().

If "abnormal", then the program exited with
an exception.  The second line of <endfile> contains an
inheritance list describing the exception.  The third
and following lines give a backtrace of the error. 

If "noexec", then the program could not be executed, and the
exception is detailed as in "abnormal".

If <endfile> is empty, then either the program called System.exit(),
or the JVM was killed with a signal. In either case, the exit
status of the JVM is the exit status of the program.

If <startfile> does not exist, then the wrapper was not executed,
indicated some fundamental problem with the Java installation.
*/

import java.io.*;
import java.lang.reflect.*;

public class CondorJavaWrapper {

	public static final int WRAPPER_ARGS = 3;
	public static final int ERROR_EXIT_CODE = 1;
	public static final int NORMAL_EXIT_CODE = 0;

	public static void main(String[] args) {

		PrintWriter out = null;

		if (args.length < WRAPPER_ARGS) {
			show_use();
			System.exit(ERROR_EXIT_CODE);
		}

		String startFile = args[0];
		String endFile = args[1];
		String jobName = args[2];

		try {
			out = new PrintWriter( new FileOutputStream(startFile) );
			out.println("started\n");
			out.flush();
			out.close();
			out = new PrintWriter( new FileOutputStream(endFile) );
		} catch( Exception e ) {
			System.err.println("CondorJavaWrapper: "+e);
			System.exit(1);
		}

		try {
			String [] jobArgs = getExecutableArgs(args);
			Class jobClass = Class.forName(jobName);
			Class [] paramTypes = { jobArgs.getClass() };
			Object [] methodArgs = { jobArgs };
			Method mainMethod = jobClass.getMethod( "main", paramTypes ); 

			mainMethod.invoke( null, methodArgs );

		} catch( InvocationTargetException ex ) {
			dump_result("abnormal",ex.getTargetException(),out);
		} catch( Throwable t ) {
			dump_result("noexec",t,out);
		}

		dump_result("normal",null,out);
		out.flush();
		out.close();
	}

	private static void dump_result( String kind, Throwable reason, PrintWriter out ) {
		out.println(kind);
		if(reason!=null) {
			print_hierarchy(reason,out);
			reason.printStackTrace(out);
			reason.printStackTrace(System.err);
		}
		out.close();
		System.exit(NORMAL_EXIT_CODE);
	}

	private static void show_use() {
		System.out.println("Use: CondorJavaWrapper <startfile> <endfile> <program> [arguments]");
	}

	private static void print_hierarchy( Object o, PrintWriter out ) {
		recurse_hierarchy(o.getClass(),out);
		out.print("\n");
	}

	private static void recurse_hierarchy( Class c, PrintWriter out ) {
		Class parent = c.getSuperclass();
		if(parent!=null) recurse_hierarchy(parent,out);
		out.print(c.getName()+" ");
	}

	private static String[] getExecutableArgs(String[] args) {
       		String [] runArgs = new String[args.length-WRAPPER_ARGS];
		for (int i=WRAPPER_ARGS; i<args.length;i++) {
			runArgs[i-WRAPPER_ARGS] = args[i];
		}
		return runArgs;
	}

}
