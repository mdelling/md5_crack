#include "cpus.h"

int num_cpus()
{
#if defined (__MACH__)
	/* Set the mib for HW_AVAILCPU. alternatively, try HW_NCPU */
	int number, mib[4] = { CTL_HW, HW_AVAILCPU, 0, 0 };
	size_t len = sizeof(number);

	/* get the number of CPUs from the system */
	sysctl(mib, 2, &number, &len, NULL, 0);

	if (number < 1) {
		mib[1] = HW_NCPU;
		sysctl(mib, 2, &number, &len, NULL, 0);

		if (number < 1)
			return 1;
	}

	return number;
#elif defined (_SC_NPROCESSORS_ONLN)
	return sysconf(_SC_NPROCESSORS_ONLN);
#endif
}
