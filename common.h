#ifndef COMMON_H
#define COMMON_H

/* Common includes */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Useful declaration macros */
#define ALIGNED __attribute__ ((aligned(16)))
#define SALIGNED static __attribute__ ((aligned(16)))
#define SCALIGNED static const __attribute__ ((aligned(16)))

/* branch prediction macros */
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

/* GCC attribute checking */
#define NONNULL(...) __attribute__((nonnull(__VA_ARGS__)))

/* A unused macro */
#define __UNUSED __attribute__((unused))
#define USE(x) ((void)(x))

#endif
