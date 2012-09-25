/******************************************************************************
 * Copyright (C) 2012 Matthew Dellinger <matthew@mdelling.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *****************************************************************************/

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
