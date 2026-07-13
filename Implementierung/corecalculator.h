#ifndef GRA26CAPSPROJECT_T105_CORECALC_H
#define GRA26CAPSPROJECT_T105_CORECALC_H

#define MIN_THREADS 1
#define MAX_THREADS 256

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h> // Unix
#endif

static int getCoreAmount(){
    int cores;
    #ifdef _WIN32
        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);
        cores = sysinfo.dwNumberOfProcessors;
    #else
        cores = sysconf(_SC_NPROCESSORS_ONLN)
    #endif

    // Error bei cores = -1
    if (cores < MIN_THREADS){
        return MIN_THREADS;
    }

    if(cores > MAX_THREADS){
        return MAX_THREADS;
    }
    return cores;
}

#endif
