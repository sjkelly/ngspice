/*
 * Author:  Holger Vogt
 * License: 3-clause BSD License
 *
 */

#include "ngspice/ngspice.h"
#include "resource.h"

#if defined(_WIN32)
#undef BOOLEAN
#include <windows.h>

#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>
#if defined(BSD) && defined(HAVE_SYS_SYSCTL_H)
#include <sys/sysctl.h>
#endif
#if defined(__APPLE__) && defined(__MACH__)
#import <mach/mach.h>
#import <mach/mach_host.h>
#endif
#else
#error "Unable to define getMemorySize( ) for an unknown OS."
#endif

#if defined(__linux__) || defined(__CYGWIN__)
static unsigned long long readProcMemInfoMemFree(void)
{
    /* Cygwin , Linux--------------------------------- */
    /* Search for string "MemFree" */
    FILE *fp;
    char buffer[2048];
    size_t bytes_read;
    char *match;
    unsigned long long mem_got;

    if ((fp = fopen("/proc/meminfo", "r")) == NULL) {
        perror("fopen(\"/proc/meminfo\")");
        return 0L;
    }

    bytes_read = fread(buffer, 1, sizeof(buffer), fp);
    fclose(fp);
    if (bytes_read == 0 || bytes_read == sizeof(buffer))
        return 0L;
    buffer[bytes_read] = '\0';
    match = strstr(buffer, "MemFree");
    if (match == NULL) /* not found */
        return 0L;
    sscanf(match, "MemFree: %llu", &mem_got);
    return mem_got * 1024L;
}
#endif

#if defined(__unix__) || defined(__unix) || defined(unix)
extern unsigned long long getTotalMemorySizeSyscall(void);
#endif

/**
 * Returns the size of available memory (RAM) in bytes.
 */
unsigned long long getAvailableMemorySize(void)
{
#if defined(_WIN32)
    /* Windows. ------------------------------------------------- */
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx( &status );
    return status.ullAvailPhys;

#elif defined(__APPLE__) && defined(__MACH__)

    mach_port_t host_port;
    mach_msg_type_number_t host_size;
    vm_size_t pagesize;

    host_port = mach_host_self();
    host_size = sizeof(vm_statistics_data_t) / sizeof(integer_t);
    host_page_size(host_port, &pagesize);

    vm_statistics_data_t vm_stat;

    if (host_statistics(host_port, HOST_VM_INFO, (host_info_t) &vm_stat,
                &host_size) != KERN_SUCCESS) {
        fprintf(stderr, "Failed to fetch vm statistics");
    }

    /* Stats in bytes */
/*    natural_t mem_used = (vm_stat.active_count + vm_stat.inactive_count +
                                    vm_stat.wire_count) * pagesize; */
    return (unsigned long long)(vm_stat.free_count * pagesize);
//    natural_t mem_total = mem_used + mem_free;

#else

#if defined(__CYGWIN__) || defined(__linux__)
    unsigned long memfree = readProcMemInfoMemFree();
    if (memfree != 0L) {
        return memfree;
    }
    // Else (if /proc is not mounted) fall through
#endif

#if defined(__unix__) || defined(__unix) || defined(unix)
    // We don't know how to get the available memory, but maybe we can get
    // the total amount of memory, which is hopefully close enough.
    return getTotalMemorySizeSyscall();
#else
    return 0L;          /* Unknown OS. */
#endif

#endif
}
