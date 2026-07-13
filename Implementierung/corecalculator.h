#ifndef GRA26CAPSPROJECT_T105_CORECALC_H
#define GRA26CAPSPROJECT_T105_CORECALC_H
#endif

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h> // Unix
#endif

static int getCoreAmount(){
    #ifdef _WIN32
        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);
        return sysinfo.dwNumberOfProcessors;
    #else
        return sysconf(_SC_NPROCESSORS_ONLN)
    #endif
}

