/* ****************************************************************************** *\

Copyright (C) 2013-2014 Intel Corporation.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
- Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.
- Neither the name of Intel Corporation nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY INTEL CORPORATION "AS IS" AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL INTEL CORPORATION BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

File Name: mfx_plugin_hive_linux.cpp

\* ****************************************************************************** */

#if defined(LINUX32) || defined(LINUX64)

#include "mfx_plugin_hive.h"
#include "mfx_library_iterator.h"
#include "mfx_dispatcher_log.h"

#include "mfx_plugin_cfg_parser.h"
#include <dirent.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>


#define TRACE_HIVE_ERROR(str, ...) DISPATCHER_LOG_ERROR((("[HIVE]: "str), __VA_ARGS__))
#define TRACE_HIVE_INFO(str, ...) DISPATCHER_LOG_INFO((("[HIVE]: "str), __VA_ARGS__))
#define TRACE_HIVE_WRN(str, ...) DISPATCHER_LOG_WRN((("[HIVE]: "str), __VA_ARGS__))

namespace 
{
    const char rootPluginPath[] = "/opt/intel/mediasdk/plugins/plugins.cfg";
    //const wchar_t rootDispatchPath[] = L"Software\\Intel\\MediaSDK\\Dispatch";
    const char pluginSubkey[] = "Plugin";
    const char TypeKeyName[] = "Type";
    const char CodecIDKeyName[] = "CodecID";
    const char GUIDKeyName[] = "GUID";
    const char PathKeyName[] = "Path";
    const char DefaultKeyName[] = "Default";
    const char PlgVerKeyName[] = "PluginVersion";
    const char APIVerKeyName[] = "APIVersion";
}

namespace 
{
#ifdef LINXU64
    const char pluginFileName[] = "FileName64";
#else
    const char pluginFileName[] = "FileName32";
#endif // LINXU64
    //do not allow store plugin in different hierarchy
    const char pluginFileNameRestrictedCharacters[] = "\\/";
    const char pluginCfgFileName[] = "plugin.cfg";
    const char pluginSearchPattern[] = "????????????????????????????????";
    const mfxU32 pluginCfgFileNameLen = 10;
    const mfxU32 pluginDirNameLen = 32;
    const mfxU32 charsPermfxU8 = 2;
    const mfxU32 slashLen = 1;
    enum 
    {
        MAX_PLUGIN_FILE_LINE = 4096
    };
}


#define alignStr() "%-14S"

namespace MFX
{

static bool isFieldMissed(mfxU32 parseMask, mfxU32 reqMask, mfxU32 field)
{
    return !(((parseMask & field) != 0) || ((reqMask & field) == 0)); //  !( reqMask & field => parseMask & field )
}

static bool CheckPluginRecord(PluginDescriptionRecord & descriptionRecord, mfxU32 foundFields, mfxU32 requiredFields)
{
    if (isFieldMissed(foundFields, requiredFields, PluginConfigParser::PARSED_TYPE))
    {
        return false;
    }        
    TRACE_HIVE_INFO(alignStr()" : %d\n", TypeKeyName, descriptionRecord.Type);

    if (isFieldMissed(foundFields, requiredFields, PluginConfigParser::PARSED_CODEC_ID)) 
    {
        TRACE_HIVE_INFO(alignStr()" : "MFXFOURCCTYPE()" \n", CodecIDKeyName, MFXU32TOFOURCC(descriptionRecord.CodecId));
    }
    else
    {
        TRACE_HIVE_INFO(alignStr()" : \n", CodecIDKeyName, "NOT REGISTERED");
    }

    if (isFieldMissed(foundFields, requiredFields, PluginConfigParser::PARSED_UID)) 
    {
        return false;
    }
    TRACE_HIVE_INFO(alignStr()" : "MFXGUIDTYPE()"\n", GUIDKeyName, MFXGUIDTOHEX(&descriptionRecord.PluginUID));

    if (isFieldMissed(foundFields, requiredFields, PluginConfigParser::PARSED_PATH)) 
    {
        TRACE_HIVE_WRN("no value for : %S\n", PathKeyName);
        return false;
    }
    TRACE_HIVE_INFO(alignStr()" : %S\n", PathKeyName, descriptionRecord.sPath);

    if (isFieldMissed(foundFields, requiredFields, PluginConfigParser::PARSED_DEFAULT)) 
    {
        return false;
    }
    TRACE_HIVE_INFO(alignStr()" : %s\n", DefaultKeyName, descriptionRecord.Default ? "true" : "false");

    if (isFieldMissed(foundFields, requiredFields, PluginConfigParser::PARSED_VERSION)) 
    {
        TRACE_HIVE_ERROR(alignStr()" : %d, which is invalid\n", PlgVerKeyName, descriptionRecord.PluginVersion);
        return false;
    } 
    TRACE_HIVE_INFO(alignStr()" : %d\n", PlgVerKeyName, descriptionRecord.PluginVersion);

    if (isFieldMissed(foundFields, requiredFields, PluginConfigParser::PARSED_API_VERSION)) 
    {
        TRACE_HIVE_ERROR(alignStr()" : %d.%d, which is invalid\n", APIVerKeyName, descriptionRecord.APIVersion.Major, descriptionRecord.APIVersion.Minor);
        return false;
    }
    TRACE_HIVE_INFO(alignStr()" : %d.%d\n", APIVerKeyName, descriptionRecord.APIVersion.Major, descriptionRecord.APIVersion.Minor);

    return true;
}

bool MFXPluginStorageBase::ConvertAPIVersion(mfxU32 APIVersion, PluginDescriptionRecord &descriptionRecord) const
{
    if (mCurrentAPIVersion.Version < descriptionRecord.APIVersion.Version ||
        mCurrentAPIVersion.Major > descriptionRecord.APIVersion.Major) 
    {
        TRACE_HIVE_ERROR(alignStr()" : %d.%d, but current MediasSDK version : %d.%d\n"
            , APIVerKeyName
            , descriptionRecord.APIVersion.Major
            , descriptionRecord.APIVersion.Minor
            , mCurrentAPIVersion.Major
            , mCurrentAPIVersion.Minor);
        return false;
    }
    return true;
}

MFXPluginsInHive::MFXPluginsInHive(int, const msdk_disp_char* msdkLibSubKey, mfxVersion currentAPIVersion)
    : MFXPluginStorageBase(currentAPIVersion)
{
    PluginConfigParser parser(rootPluginPath);
    int numPlugins = parser.GetPluginCount();

    if (numPlugins < 0)
    {
        TRACE_HIVE_ERROR("no plugin records found in %s\n", rootPluginPath);
        return;
    }
    
    try 
    {
        resize(numPlugins);
    }
    catch (...) {
        TRACE_HIVE_ERROR("new PluginDescriptionRecord[%d] threw an exception: \n", numPlugins);
        return;
    }

    for (int index = 0; index < numPlugins; index++, parser.AdvanceToNextPlugin())
    {
        PluginDescriptionRecord descriptionRecord;
        try 
        {
            char pluginName[MAX_PLUGIN_NAME];
            bool nameRes = parser.GetCurrentPluginName(pluginName);

            mfxU32 foundFields = 0;

            bool infoRes = parser.ParsePluginParams(descriptionRecord, foundFields);
            if (!nameRes || !infoRes)
            {
                TRACE_HIVE_WRN("unable to parse section # %d in %s\n", index, rootPluginPath);
                continue;
            }
            TRACE_HIVE_INFO("Found Plugin: %S\n", pluginName);

            mfxU32 reqs =  PluginConfigParser::PARSED_VERSION
                         | PluginConfigParser::PARSED_API_VERSION
                         | PluginConfigParser::PARSED_PATH
                         | PluginConfigParser::PARSED_UID;
//                       | PluginConfigParser::PARSED_NAME
//                       | PluginConfigParser::PARSED_TYPE
//                       | PluginConfigParser::PARSED_CODEC_ID
//                       | PluginConfigParser::PARSED_DEFAULT;

            if (CheckPluginRecord(descriptionRecord, foundFields, reqs))
            {
                if (!ConvertAPIVersion(0, descriptionRecord))
                    continue;
                (*this)[index] = descriptionRecord;
            }
            else
            {
                TRACE_HIVE_WRN("Registration of plugin %s found, but missed some fields (mask 0x%x)\n", pluginName, reqs ^ foundFields);
            }
        }
        catch (...)
        {
            TRACE_HIVE_ERROR("operator[](%d) = descriptionRecord; - threw exception \n", index);
        }
    }
}


static int plugin_name_filter(const struct dirent * name)
{
    if (pluginDirNameLen != strlen(name->d_name))
        return 0;
    for (int i = 0; i < pluginDirNameLen; i++)
    {
        if (!isxdigit(name->d_name[i]))
            return 0;
    }
    
    return 1;
}


MFXPluginsInFS::MFXPluginsInFS(mfxVersion currentAPIVersion)
    : MFXPluginStorageBase(currentAPIVersion)
    , mIsVersionParsed()
    , mIsAPIVersionParsed()
{
    char selfName[MAX_PLUGIN_PATH];
    ssize_t nRead = readlink("/proc/self/exe", selfName, sizeof(selfName) - 1);
    if (nRead < 0)
    {
        TRACE_HIVE_ERROR("readlink(\"/proc/self/exe\") reported an error: %d\n", errno);
        return;
    }
    selfName[nRead] = '\0';

    char *lastSlashPos = strrchr(selfName, '/');
    if (!lastSlashPos) {
        lastSlashPos = selfName;
    }
    mfxU32 executableDirLen = (mfxU32)(lastSlashPos - selfName) + slashLen;
    if (executableDirLen + pluginDirNameLen + pluginCfgFileNameLen >= MAX_PLUGIN_PATH) 
    {
        TRACE_HIVE_ERROR("MAX_PLUGIN_PATH which is %d, not enough to locate plugin path\n", MAX_PLUGIN_PATH);
        return;
    }
    // strncpy(lastSlashPos + slashLen, pluginSearchPattern, MAX_PLUGIN_PATH - executableDirLen);
    *lastSlashPos = 0;


    dirent **namelist;
    int n = scandir(selfName, &namelist, plugin_name_filter, alphasort);
    if (n < 0)
    {
        TRACE_HIVE_ERROR("Error %d scanning application directory %s\n", errno, selfName);
    }
    else 
    {
        for (int i = 0; i < n; i++)
        {
            PluginDescriptionRecord descriptionRecord;
            descriptionRecord.onlyVersionRegistered = true;
            char cfgName[MAX_PLUGIN_PATH];
            snprintf(cfgName, sizeof(cfgName), "%s/%s/%s", selfName, namelist[i]->d_name, pluginCfgFileName);
            if ( strlen(selfName) + strlen("/") + strlen(namelist[i]->d_name) >= MAX_PLUGIN_PATH)
            {
                TRACE_HIVE_ERROR("buffer of MAX_PLUGIN_PATH characters which is %d, not enough to store plugin directory path: %s/%s\n",
                    MAX_PLUGIN_PATH, selfName, namelist[i]->d_name);
            }

            strcpy(descriptionRecord.sPath, selfName);
            strcpy(descriptionRecord.sPath + strlen(descriptionRecord.sPath), "/");
            strcpy(descriptionRecord.sPath + strlen(descriptionRecord.sPath), namelist[i]->d_name);

            if (!parseGUID(namelist[i]->d_name, descriptionRecord.PluginUID.Data))
            {
                TRACE_HIVE_ERROR("directory name %s is not valid guid\n", namelist[i]->d_name);
                continue;
            }
            free(namelist[i]);
                      
            PluginConfigParser parser(cfgName);
            int numPlugins = parser.GetPluginCount();

            if (numPlugins < 0)
            {
                TRACE_HIVE_ERROR("no plugin records found in %s\n", cfgName);
                continue;
            }
            if (numPlugins > 1)
            {
                TRACE_HIVE_ERROR("too many plugin records found in %s\n", cfgName);
                continue;
            }


            try 
            {
                char pluginName[MAX_PLUGIN_NAME];
                bool nameRes = parser.GetCurrentPluginName(pluginName);
                if (!nameRes)
                {
                    TRACE_HIVE_WRN("unable to parse plugin name from %s\n", cfgName);
                }

                mfxU32 foundFields = PluginConfigParser::PARSED_UID;

                bool infoRes = parser.ParsePluginParams(descriptionRecord, foundFields);
                if (!infoRes)
                {
                    TRACE_HIVE_WRN("unable to parse plugin information in %s\n", cfgName);
                    continue;
                }
                TRACE_HIVE_INFO("Found Plugin: %S\n", pluginName);

                mfxU32 reqs =  PluginConfigParser::PARSED_VERSION
                             | PluginConfigParser::PARSED_API_VERSION
                             | PluginConfigParser::PARSED_PATH
                             | PluginConfigParser::PARSED_UID;

                if (CheckPluginRecord(descriptionRecord, foundFields, reqs))
                {
                    if (!ConvertAPIVersion(0, descriptionRecord))
                        continue;
                    push_back(descriptionRecord);
                }
            }
            catch (...)
            {
                TRACE_HIVE_ERROR("push_back(descriptionRecord) - threw exception \n", index);
            }

        }
        free(namelist);
    }
}

} // namespace MFX


#endif
