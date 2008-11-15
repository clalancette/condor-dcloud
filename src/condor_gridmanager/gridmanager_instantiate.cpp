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



#include "condor_common.h"
#include "proc.h"
#include "HashTable.h"
#include "list.h"
#include "simplelist.h"

#include "gridmanager.h"
#include "amazonresource.h"	// added by fangcao
#include "basejob.h"
#include "condorjob.h"
#include "condorresource.h"
#include "gahp-client.h"
#include "globusjob.h"
#include "globusresource.h"
#include "gt4job.h"
#include "gt4resource.h"
#include "infnbatchjob.h"
#include "mirrorjob.h"
#include "mirrorresource.h"
#include "nordugridjob.h"
#include "nordugridresource.h"
#include "unicorejob.h"
#include "creamjob.h"
#include "creamresource.h"

template class List<NordugridJob>;
template class Item<NordugridJob>;
template class HashTable<HashKey, NordugridResource *>;
template class HashTable<HashKey, AmazonResource *>;		// added by fangcao
template class HashBucket<HashKey, NordugridResource *>;

#if defined(ORACLE_UNIVERSE)
#   include "oraclejob.h"
#   include "oraclereosurce.h"

    template class HashTable<HashKey, OracleJob *>;
    template class HashBucket<HashKey, OracleJob *>;
    template class List<OracleJob>;
    template class Item<OracleJob>;
#endif

#include "proxymanager.h"

template class HashTable<PROC_ID, BaseJob *>;
template class HashBucket<PROC_ID, BaseJob *>;
template class List<BaseJob>;
template class Item<BaseJob>;
template class HashTable<HashKey, BaseJob *>;
template class HashBucket<HashKey, BaseJob *>;

template class HashTable<HashKey, CondorResource *>;
template class HashBucket<HashKey, CondorResource *>;
template class HashTable<HashKey, CondorResource::ScheddPollInfo *>;
template class HashBucket<HashKey, CondorResource::ScheddPollInfo *>;
template class List<CondorJob>;
template class Item<CondorJob>;

template class HashTable<int,GahpClient*>;
template class ExtArray<Gahp_Args*>;
template class Queue<int>;
template class HashTable<HashKey, GahpServer *>;
template class HashBucket<HashKey, GahpServer *>;
template class HashTable<HashKey, GahpProxyInfo *>;
template class HashBucket<HashKey, GahpProxyInfo *>;
template class SimpleList<PROC_ID>;
template class SimpleListIterator<PROC_ID>;
template class SimpleListIterator<int>;

template class HashTable<HashKey, GlobusJob *>;
template class HashBucket<HashKey, GlobusJob *>;
template class HashTable<HashKey, GlobusResource *>;
template class HashBucket<HashKey, GlobusResource *>;
template class List<GlobusJob>;
template class Item<GlobusJob>;

template class HashTable<HashKey, GT4Resource *>;
template class HashBucket<HashKey, GT4Resource *>;
template class List<GT4Job>;
template class Item<GT4Job>;

template class HashTable<HashKey, CreamResource *>;
template class HashBucket<HashKey, CreamResource *>;

template class HashTable<HashKey, MirrorJob *>;
template class HashBucket<HashKey, MirrorJob *>;
template class HashTable<HashKey, MirrorResource *>;
template class HashBucket<HashKey, MirrorResource *>;

template class HashTable<HashKey, UnicoreJob *>;
template class HashBucket<HashKey, UnicoreJob *>;

template class HashTable<HashKey, Proxy *>;
template class HashBucket<HashKey, Proxy *>;
template class HashTable<HashKey, ProxySubject *>;
template class HashBucket<HashKey, ProxySubject *>;
template class List<Proxy>;
template class Item<Proxy>;
template class SimpleList<MyProxyEntry*>;

template class HashTable<HashKey, GridftpServer *>;
template class HashBucket<HashKey, GridftpServer *>;