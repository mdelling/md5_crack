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
