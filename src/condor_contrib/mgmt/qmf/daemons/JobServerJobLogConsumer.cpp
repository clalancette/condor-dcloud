/***************************************************************
 *
 * Copyright (C) 2009-2011 Red Hat, Inc.
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


#include "condor_common.h"
#include "condor_debug.h"
#include "condor_exprtype.h"
#include "condor_attributes.h"
#include "condor_astbase.h"
#include "condor_parser.h"
#include "condor_qmgr.h"

#include "ClassAdLogReader.h"

#include "JobServerJobLogConsumer.h"

#include "Globals.h"

#include "HistoryProcessingUtils.h"

#define IS_JOB(key) ((key) && '0' != (key)[0])

JobServerJobLogConsumer::JobServerJobLogConsumer()
{ }

JobServerJobLogConsumer::~JobServerJobLogConsumer()
{ }

void
JobServerJobLogConsumer::Reset()
{
	// When deleting jobs, to avoid problems with jobs referencing
	// deleted clusters, we must be sure to delete the clusters
	// last

	dprintf(D_FULLDEBUG, "JobServerJobLogConsumer::Reset() - deleting jobs and submissions\n");

	for (JobCollectionType::iterator i = g_jobs.begin();
	     g_jobs.end() != i; i++) {
		// Skip clusters for now
		if ('0' != (*i).second->GetKey()[0]) {
			delete (*i).second;
			g_jobs.erase(i);
		}
	}

	for (JobCollectionType::iterator i = g_jobs.begin();
		  g_jobs.end() != i; i++) {
		     delete (*i).second;
		     g_jobs.erase(i);
	}

	for (SubmissionCollectionType::iterator i = g_submissions.begin();
		       g_submissions.end() != i; i++) {
			  delete (*i).second;
			  g_submissions.erase(i);
	}

	// due to the shared use of g_jobs
	// a JobLogReader->Reset() might cause
	// us to reload our history
	init_history_files();

}

bool
JobServerJobLogConsumer::NewClassAd(const char *_key,
									const char */*type*/,
									const char */*target*/)
{

	const char* key_dup = strdup(_key);

	dprintf(D_FULLDEBUG, "JobServerJobLogConsumer::NewClassAd processing _key='%s'\n", _key);

	// ignore the marker
	if (strcmp(_key,"0.0") == 0) {
	  return true;
	}

	if ('0' == _key[0]) {
		// Cluster ad
		if (g_jobs.end() == g_jobs.find(_key)) {
			Job* new_cluster_job = new Job(key_dup);
			new_cluster_job->SetImpl(new ClusterJobImpl(key_dup));
			g_jobs[key_dup] = new_cluster_job;
		}
	} else {
		// Proc ad

		// first see if some other proc job is here
		// ie history
		if (g_jobs.end() != g_jobs.find(_key)) {
			return true;
		}

		PROC_ID proc = getProcByString(_key);
		MyString cluster_key;

		cluster_key.sprintf("0%d.-1", proc.cluster);

		const char *cluster_dup = cluster_key.StrDup();
		JobCollectionType::const_iterator element = g_jobs.find(cluster_dup);
        ClusterJobImpl* cluster_impl = NULL;

		// TODO this code assumes that we will always get the parent 
		// classad before its child from the job log...this is not strictly
		// guaranteed (e.g., compressed log?)
		if (g_jobs.end() == element) {
			// didn't find an existing job so create a new one
			Job* new_cluster_job = new Job(cluster_dup);
            cluster_impl = new ClusterJobImpl(cluster_dup);
            new_cluster_job->SetImpl(cluster_impl);
            g_jobs[cluster_dup] = new_cluster_job;
		} else {
			// found an existing job - we'll assume it is the cluster parent
			cluster_impl = static_cast<ClusterJobImpl*>((*element).second->GetImpl());
		}

        Job* new_proc_job = new Job(key_dup);
        new_proc_job->SetImpl(new LiveJobImpl(key_dup, cluster_impl));
        g_jobs[key_dup] = new_proc_job;

//		if (cluster_job) {
//			ClassAd ad;
//			cluster_job->GetFullAd(ad);
//			dprintf(D_FULLDEBUG, "JobServerJobLogConsumer::NewClassAd found a parent ClassAd from cluster...\n");
//			ad.dPrint(D_FULLDEBUG|D_NOHEADER);
//		}

	}

	return true;
}

bool
JobServerJobLogConsumer::DestroyClassAd(const char *_key)
{
	// ignore the marker
	if (strcmp(_key,"0.0") == 0) {
	  return true;
	}

   dprintf ( D_FULLDEBUG, "JobServerJobLogConsumer::DestroyClassAd - key '%s'\n", _key);
    JobCollectionType::iterator g_element = g_jobs.find(_key);

    if (g_jobs.end() == g_element) {
        dprintf(D_ALWAYS,
                "error reading %s: no such job found for key '%s'\n",
                m_reader->GetClassAdLogFileName(), _key);
        return false;
    }

    Job* job = (*g_element).second;
    // Destroy will figure out the submission decrement
    if (job->Destroy()) {
            delete job;
            job = NULL;
            g_jobs.erase(g_element);
    }

    return true;
}

bool
JobServerJobLogConsumer::SetAttribute(const char *_key,
									  const char *_name,
									  const char *_value)
{
	// ignore the marker
	if (strcmp(_key,"0.0") == 0) {
	  return true;
	}

	if (0 == strcmp(_name,"NextClusterNum") ) {
		// skip over these
		//dprintf(D_FULLDEBUG, "%s: skipping job entry '%s' for '%s = %s'\n",
		//	m_reader->GetJobLogFileName(), _key, _name, _value);
		return true;
	}

    JobCollectionType::const_iterator g_element = g_jobs.find(_key);

	if (g_jobs.end() == g_element) {
		dprintf(D_ALWAYS,
				"error reading %s: no such job '%s' for '%s = %s'\n",
                m_reader->GetClassAdLogFileName(), _key, _name, _value);
		return false;
	}

	(*g_element).second->Set(_name, _value);

	return true;
}

bool
JobServerJobLogConsumer::DeleteAttribute(const char *_key,
										 const char *_name)
{
	// ignore the marker
	if (strcmp(_key,"0.0") == 0) {
	  return true;
	}

	JobCollectionType::const_iterator g_element = g_jobs.find(_key);

	if (g_jobs.end() == g_element) {
		dprintf(D_ALWAYS,
				"error reading %s: no such job '%s' for 'delete %s'\n",
                m_reader->GetClassAdLogFileName(), _key, _name);
		return false;
	}

	(*g_element).second->Remove(_name);

	return true;
}
