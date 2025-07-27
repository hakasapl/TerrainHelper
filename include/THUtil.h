#pragma once

#include <windows.h>
#include <psapi.h>

namespace THUtil
{
    /**
     * @brief Checks if ENB is loaded or not (Comes from CS implementation)
     *
     * @return bool true if loaded, false otherwise
     */
    [[nodiscard]] static bool IsENBLoaded()
    {
        DWORD cb = 1000 * sizeof(HMODULE);
        DWORD cbNeeded = 0;
        HMODULE enbmodule = NULL;
        HMODULE hmodules[1000];
        HANDLE hproc = GetCurrentProcess();
        for (long i = 0; i < 1000; i++)
        {
            hmodules[i] = NULL;
        }

        // Find the proper library using the existance of an exported function, because several with the same name may exist
        if (EnumProcessModules(hproc, hmodules, cb, &cbNeeded))
        {
            long count = cbNeeded / sizeof(HMODULE);
            for (long i = 0; i < count; i++)
            {
                if (hmodules[i] == NULL)
                {
                    break;
                }

                void *func = (void *)GetProcAddress(hmodules[i], "ENBGetSDKVersion");
                if (func)
                {
                    enbmodule = hmodules[i];
                    break;
                }
            }
        }

        return enbmodule != nullptr;
    }
};
