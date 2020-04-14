/*
 * This file is part of libbluray
 * Copyright (C) 2011  VideoLAN
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#if defined(WINAPI_FAMILY_PARTITION) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP) && !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#define COBJMACROS
#define INITGUID
#include <winstring.h>
#include <windows.storage.h>
#include <roapi.h>
#endif

#include "dirs.h"

#include "util/logging.h"

#include <stdio.h>
#include <string.h>

#include <windows.h>
#include <shlobj.h>
#include <limits.h>


#if defined(WINAPI_FAMILY_PARTITION) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP) && !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
DEFINE_GUID(IID_IStorageItem, 0x4207a996, 0xca2f, 0x42f7, 0xbd, 0xe8, 0x8b, 0x10, 0x45, 0x7a, 0x7f, 0x30);
DEFINE_GUID(IID_IApplicationDataStatics, 0x5612147b, 0xe843, 0x45e3, 0x94, 0xd8, 0x06, 0x16, 0x9e, 0x3c, 0x8e, 0x17);
DEFINE_GUID(IID_IKnownFoldersStatics, 0x5a2a7520, 0x4802, 0x452d, 0x9a, 0xd9, 0x43, 0x51, 0xad, 0xa7, 0xec, 0x35);

typedef __x_ABI_CWindows_CStorage_CIStorageFolder           IStorageFolder;
typedef __x_ABI_CWindows_CStorage_CIApplicationDataStatics  IApplicationDataStatics;
typedef __x_ABI_CWindows_CStorage_CIApplicationData         IApplicationData;
typedef __x_ABI_CWindows_CStorage_CIApplicationData3        IApplicationData3;
typedef __x_ABI_CWindows_CStorage_CIKnownFoldersStatics     IKnownFoldersStatics;
typedef __x_ABI_CWindows_CStorage_CIStorageItem             IStorageItem;

#define IApplicationDataStatics_get_Current         __x_ABI_CWindows_CStorage_CIApplicationDataStatics_get_Current
#define IApplicationData_get_RoamingFolder          __x_ABI_CWindows_CStorage_CIApplicationData_get_RoamingFolder
#define IApplicationData_get_LocalFolder            __x_ABI_CWindows_CStorage_CIApplicationData_get_LocalFolder
#define IApplicationData3_get_SharedLocalFolder     __x_ABI_CWindows_CStorage_CIApplicationData3_get_SharedLocalFolder
#define IStorageItem_get_Path                       __x_ABI_CWindows_CStorage_CIStorageItem_get_Path
#define IStorageItem_Release                        __x_ABI_CWindows_CStorage_CIStorageItem_Release
#define IStorageFolder_QueryInterface               __x_ABI_CWindows_CStorage_CIStorageFolder_QueryInterface
#define IStorageFolder_Release                      __x_ABI_CWindows_CStorage_CIStorageFolder_Release
#define IApplicationDataStatics_Release             __x_ABI_CWindows_CStorage_CIApplicationDataStatics_Release
#define IKnownFoldersStatics_Release                __x_ABI_CWindows_CStorage_CIKnownFoldersStatics_Release
#define IApplicationData_Release                    __x_ABI_CWindows_CStorage_CIApplicationData_Release

typedef enum
{
    SHGFP_TYPE_CURRENT = 0,
    SHGFP_TYPE_DEFAULT = 1,
} SHGFP_TYPE;

#ifndef CSIDL_FLAG_CREATE
#   define CSIDL_FLAG_CREATE               0x8000
#endif
#ifndef CSIDL_APPDATA
#   define CSIDL_APPDATA                   0x001a
#endif
#ifndef CSIDL_LOCAL_APPDATA
#   define CSIDL_LOCAL_APPDATA             0x001c
#endif
#ifndef CSIDL_COMMON_APPDATA
#   define CSIDL_COMMON_APPDATA            0x0023
#endif

static HRESULT winRTSHGetFolderPathW(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPWSTR pszPath)
{
    HRESULT hr = E_FAIL;
    IStorageFolder *folder;

    if (dwFlags != SHGFP_TYPE_CURRENT)
        return E_NOTIMPL;

    folder = NULL;
    csidl &= ~0xFF00; /* CSIDL_FLAG_MASK */

    // Only supports CSIDL_APPDATA, CSIDL_LOCAL_APPDATA, CSIDL_COMMON_APPDATA
    IApplicationDataStatics *appDataStatics = NULL;
    IApplicationData *appData = NULL;
    IApplicationData3 *appData3 = NULL;
    static const WCHAR *className = L"Windows.Storage.ApplicationData";
    const UINT32 clen = wcslen(className);

    HSTRING hClassName = NULL;
    HSTRING_HEADER header;
    hr = WindowsCreateStringReference(className, clen, &header, &hClassName);
    if (FAILED(hr))
        goto end_appdata;

    hr = RoGetActivationFactory(hClassName, &IID_IApplicationDataStatics, (void**)&appDataStatics);

    if (FAILED(hr))
        goto end_appdata;

    if (!appDataStatics) {
        goto end_appdata;
    }

    hr = IApplicationDataStatics_get_Current(appDataStatics, &appData);

    if (FAILED(hr))
        goto end_appdata;

    if (!appData)
        goto end_appdata;

    switch (csidl) {
        case CSIDL_APPDATA:
            hr = IApplicationData_get_RoamingFolder(appData, &folder);
            break;
        case CSIDL_LOCAL_APPDATA:
            hr = IApplicationData_get_LocalFolder(appData, &folder);
            break;
        case CSIDL_COMMON_APPDATA:
            //Enable the policy e.g.by creating a REG_DWORD value called AllowSharedLocalAppData under 
            //   HKLM\SOFTWARE\Policies\Microsoft\Windows\CurrentVersion\AppModel\StateManager and set it to 1.
            hr = IApplicationData3_get_SharedLocalFolder((IApplicationData3*)appData, &folder);
            break;
        default:
            hr = E_NOTIMPL;
    }

end_appdata:
    WindowsDeleteString(hClassName);
    if (appDataStatics)
        IApplicationDataStatics_Release(appDataStatics);
    if (appData)
        IApplicationData_Release(appData);

    if (SUCCEEDED(hr) && (folder != NULL)) {
        PCWSTR pszPathTemp;
        IStorageItem *item = NULL;
        HSTRING path = NULL;
        hr = IStorageFolder_QueryInterface(folder, &IID_IStorageItem, (void**)&item);
        if (FAILED(hr))
            goto end_folder;
        hr = IStorageItem_get_Path(item, &path);
        if (FAILED(hr))
            goto end_folder;
        pszPathTemp = WindowsGetStringRawBuffer(path, NULL);
        wcscpy(pszPath, pszPathTemp);
end_folder:
        WindowsDeleteString(path);
        IStorageFolder_Release(folder);
        if (item)
            IStorageItem_Release(item);
    }

    return hr;
}
#define SHGetFolderPathW winRTSHGetFolderPathW
#endif

char *file_get_config_home(void)
{
    return file_get_data_home();
}

char *file_get_data_home(void)
{
    wchar_t wdir[MAX_PATH];

    /* Get the "Application Data" folder for the user */
    if (S_OK == SHGetFolderPathW(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE,
                                 NULL, SHGFP_TYPE_CURRENT, wdir)) {
        int len = WideCharToMultiByte (CP_UTF8, 0, wdir, -1, NULL, 0, NULL, NULL);
        char *appdir = malloc(len);
        if (appdir) {
            WideCharToMultiByte (CP_UTF8, 0, wdir, -1, appdir, len, NULL, NULL);
        }
        return appdir;
    }

    BD_DEBUG(DBG_FILE, "Can't find user configuration directory !\n");
    return NULL;
}

char *file_get_cache_home(void)
{
    return file_get_data_home();
}

const char *file_get_config_system(const char *dir)
{
    static char *appdir = NULL;
    wchar_t wdir[MAX_PATH];

    if (!dir) {
        // first call

        if (appdir)
            return appdir;

        /* Get the "Application Data" folder for all users */
        if (S_OK == SHGetFolderPathW(NULL, CSIDL_COMMON_APPDATA | CSIDL_FLAG_CREATE,
                    NULL, SHGFP_TYPE_CURRENT, wdir)) {
            int len = WideCharToMultiByte (CP_UTF8, 0, wdir, -1, NULL, 0, NULL, NULL);
            appdir = malloc(len);
            if (appdir) {
                WideCharToMultiByte (CP_UTF8, 0, wdir, -1, appdir, len, NULL, NULL);
            }
            return appdir;
        } else {
            BD_DEBUG(DBG_FILE, "Can't find common configuration directory !\n");
            return NULL;
        }
    } else {
        // next call
        return NULL;
    }

    return dir;
}
