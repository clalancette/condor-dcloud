/*
 * Copyright 2008 Red Hat, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "condor_common.h"

#include "MgmtScheddPlugin.h"

MgmtScheddPlugin *scheddPluginInstance;

#ifdef WIN32
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved )
{
    switch ( ul_reason_for_call )
    {
        case DLL_PROCESS_ATTACH:
            scheddPluginInstance = new MgmtScheddPlugin();
            dprintf(D_FULLDEBUG, "WINDOWS loading MgmtScheddPlugin\n");
        //case DLL_THREAD_ATTACH:
        //case DLL_THREAD_DETACH:
        //case DLL_PROCESS_DETACH:
            break;
    }

    return TRUE;
}

#else

void
__attribute__ ((constructor))
init(void)
{
    scheddPluginInstance = new MgmtScheddPlugin();
}

#endif
