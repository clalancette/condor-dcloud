<?php

# This function is used to construct the timestamp portion of Bonsai URL.
function parseTimeStamp($ts) {
  list($date, $time) = explode(' ', $ts);
  list($y, $m, $d) = explode('-', $date);
  list($h, $min, $sec) = explode(':', $time);
  $date = join('%2F', array($m, $d, $y));
  $time = join('%3A', array($h, $min, $sec));
  return "$date+$time";
}

# This function accepts a build id and return a Bonsai URL showing the diff
# between the version being built and the last successful build of the same
# branch on the same platform and submitted by the same user (on success).
# On failure it returns the empty string.
function getLastGoodBuild($id, $debug) {
# This section is configurable - add/remove branches as needed.
  $branches['V6_8-branch'] = 1;
  $branches['V6_9-trunk'] = 1;

  # echo "DEBUG: runid --> $id<br>\n";
  $query1 = "SELECT cvs_tag from Method_cvs WHERE runid=$id";
  $result1 = mysql_query($query1) or die ("Query failed : " . mysql_error());
  $res1 = mysql_fetch_array($result1);
  $tag = $res1['cvs_tag'];

  if (! $tag) {
    if ($debug) echo "DEBUG: could not find cvs_tag for build id $id.<br>\n";
    return '';
  }

  # Compute the timestamp when this build ended.
  $query1 = "SELECT finish, platform from Task WHERE runid=$id";
  $result1 = mysql_query($query1) or die ("Query failed : " . mysql_error());
  $res1 = mysql_fetch_array($result1);
  if (! $res1) {
    if ($debug )echo "DEBUG: could not find finish timestamp for build id $id.<br>\n";
    return '';
  }
  $finish = $res1['finish'];
  $platform = $res1['platform'];
  if ($debug) echo "DEBUG: platform --> $platform<br>\n";
  $end_ts = parseTimeStamp($finish);

  $query2 = "SELECT user from Run WHERE runid=$id";
  $result2 = mysql_query($query2) or die ("Query failed : " . mysql_error());
  $res2 = mysql_fetch_array($result2);
  if (! $res2) return '';
  $user = $res2['user'];
  if ($debug) echo "DEBUG: user --> $user<br>\n";

  if ($debug) echo "DEBUG: cvs_tag is $tag.<br>\n<br>\n";
  # preg_match('/^(.*?)-(\d+)-(\d+)-(\d+)$/', $tag, $parts) or 
  #   die("Cannot parse out date from $tag.");
  if (! preg_match('/^(.*?)-(\d+)-(\d+)-(\d+)$/', $tag, $parts)) return '';
    
  $prefix = $parts[1];
  if ($debug) echo "DEBUG: prefix --> $prefix<br>\n";
  $branch = preg_replace('/^BUILD-/', '', $prefix);
  if ($debug) echo "DEBUG: branch --> '$branch'<br>\n";
  $present = $branches[$branch];
  if ($debug) echo "DEBUG: present -> $present.<br>\n";
  if ($debug) echo "DEBUG: ", $branches['V6_8-branch'], "<br>\n";
  if ($debug) echo "DEBUG: ", $branches['V6_9-trunk'], "<br>\n";
  
  if ($branches[$branch] != 1) return '';
  $year = $parts[2];
  $month = $parts[3];
  $day = $parts[4];
  $y = $year; $m = $month; $d = $day;

  while (1) {
    $ts = mktime(0, 0, 0, $m, $d - 1, $y);
    $dat = date('j\-n\-Y', $ts);
    preg_match('/^(\d+)-(\d+)-(\d+)$/', $dat, $dates) or die ("Could not parse '$dat'");
    $y = $dates[3];
    $m = $dates[2];
    $d = $dates[1];
    $newtag = "$prefix-$y-$m-$d";
    if ($debug) echo "DEBUG: looking for tag->$newtag, platform->$platform, user->$user.<br>\n";

    $query3 = "SELECT Task.start, Task.runid as runid FROM Method_cvs LEFT JOIN Run ON Method_cvs.runid=Run.runid ";
    $query3 .= "LEFT JOIN Task ON Method_cvs.runid=Task.runid ";
    $query3 .= "WHERE platform='$platform' AND Run.result IS NOT NULL AND Run.result != 0 ";
    $query3 .= "AND Method_cvs.cvs_tag='$newtag'";
    $result3 = mysql_query($query3) or die ("Query failed : " . mysql_error());
    $res3 = mysql_fetch_array($result3);
    if (! $res3) break;
    $start = $res3['start'];
    $runid = $res3['runid'];
    if ($debug) echo "DEBUG: last successful build: runid => $runid, tag => $newtag.<br>\n";
    if ($debug) echo "DEBUG: start => $start.<br>\n";
    $start_ts = parseTimeStamp($start);
    return "http://bonsai.cs.wisc.edu/bonsai/cvsquery.cgi?treeid=default&module=all&branch=$branch&branchtype=match&dir=&file=&filetype=match&who=&whotype=match&sortby=Date&hours=2&date=explicit&mindate=$start_ts&maxdate=$end_ts&cvsroot=%2Fp%2Fcondor%2Frepository%2FCONDOR_SRC";

  }
  return (false);
} 

function get_queue_for_nmi_platform($platform, $type) {
  // For the purposes of this script crosstype => type
  if($type == "crosstest") $type = "test";

  $platform_without_prefix = preg_replace("/nmi:/", "", $platform);
  $output = `/usr/local/condor/bin/condor_q -global -const 'nmi_target_platform=="$platform_without_prefix" && nmi_run_type=="$type"' -format '%-2s ' 'ifThenElse(JobStatus==0,"U",ifThenElse(JobStatus==1,"I",ifThenElse(JobStatus==2,"R",ifThenElse(JobStatus==3,"X",ifThenElse(JobStatus==4,"C",ifThenElse(JobStatus==5,"H",ifThenElse(JobStatus==6,"E",string(JobStatus))))))))' -format "%6d " ClusterId -format " %-14s " Owner -format '%-11s\n' 'formatTime(QDate,"%0m/%d %H:%M")'`;
  $queue_contents = split("\n", $output);
  $depth = sizeof($queue_contents) - 1;
  $output = "<table><tr><th>State</th><th>ID</th><th>Owner</th><th>Submitted</th></tr>\n";
  $has_running_job = 0;
  foreach ($queue_contents as $line) {
    $items = preg_split("/\s+/", $line);
    if(sizeof($items) == 5) {
      $style = "background-color:#FFFFAA; text-decoration:none;";
      if($items[0] == "R") {
	$style = "background-color:#0097C5;";
	$has_running_job++;
      }
      elseif($items[0] == "H") {
	$style = "background-color:#A1A1A1;";
      }
      $output .= "<tr style=\"$style\"><td style=\"text-align:center\">$items[0]</td><td>$items[1]</td><td>$items[2]</td><td>$items[3]&nbsp;$items[4]</td></tr>\n";
    }
  }
  $output .= "</table>\n";

  $note = "";
  if($has_running_job == 0) {
    $note = "*";
  }

  $ret = Array();
  $ret[0] = $depth;
  $ret[1] = "<br /><span class=\"link\"><a href=\"javascript: void(0)\" style=\"text-decoration:none;\">Q Depth: $depth$note<span>$output</span></a></span>";
  return $ret;
}

?>

