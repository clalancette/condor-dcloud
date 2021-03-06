#! /usr/bin/env perl
##**************************************************************
##
## Copyright (C) 1990-2007, Condor Team, Computer Sciences Department,
## University of Wisconsin-Madison, WI.
## 
## Licensed under the Apache License, Version 2.0 (the "License"); you
## may not use this file except in compliance with the License.  You may
## obtain a copy of the License at
## 
##    http://www.apache.org/licenses/LICENSE-2.0
## 
## Unless required by applicable law or agreed to in writing, software
## distributed under the License is distributed on an "AS IS" BASIS,
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
## See the License for the specific language governing permissions and
## limitations under the License.
##
##**************************************************************

my $arg = $ARGV[0];
my $basefile = $ARGV[1];

my $new = $basefile . ".new";
my $old = $basefile;

open(OLDOUT, "<$old");
open(NEWOUT, ">$new");
while(<OLDOUT>)
{
	fullchomp($_);
	print NEWOUT "$_\n";
}
print NEWOUT "$arg\n";

close(OLDOUT);
close(NEWOUT);
system("mv $new $old");
print "Job $arg done\n";
exit(0);


sub fullchomp
{
	push (@_,$_) if( scalar(@_) == 0);
	foreach my $arg (@_) {
		$arg =~ s/\012+$//;
		$arg =~ s/\015+$//;
	}
	return(0);
}

