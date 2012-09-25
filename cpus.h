/* Provides num_cpus() implementations for various platforms */
#ifndef CPUS_H
#define CPUS_H

#include "common.h"

#if defined (__MACH__)
#include <sys/param.h>
#include <sys/sysctl.h>
#endif

int num_cpus(void);

#endif
