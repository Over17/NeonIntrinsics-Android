#include <errno.h>
#include <android/log.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "CpuThreadControl.h"

void SetCurrentThreadAffinity(unsigned int affinity)
{
	__android_log_print(ANDROID_LOG_INFO, "NeonIntrinsics", "Setting thread affinity to %d...\n", affinity);
    int result = syscall(__NR_sched_setaffinity, gettid(), sizeof(affinity), &affinity);
	if (result)
	{
		__android_log_print(ANDROID_LOG_ERROR, "NeonIntrinsics", "Error setting thread affinity, errno = %d\n", errno);
	}
}

unsigned int GetLittleCoreAffinity()
{
	// Core 0 should be little
	return 1;
}

unsigned int GetBigCoreAffinity()
{
	int coreCount = (int)sysconf(_SC_NPROCESSORS_CONF);
	__android_log_print(ANDROID_LOG_INFO, "NeonIntrinsics", "CPU Core count = %d\n", coreCount);
	// Return mask containing one (last) core which should be the biggest
	return 1 << (coreCount - 1);
}
