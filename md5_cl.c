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

#include "md5_cl.h"

cl_device_id device_id = NULL;
cl_context context = NULL;
cl_command_queue command_queue = NULL;
cl_mem memobj1 = NULL;
cl_mem memobj2 = NULL;
cl_program program = NULL;
cl_kernel kernel = NULL;
cl_platform_id platform_id = NULL;
cl_uint ret_num_devices;
cl_uint ret_num_platforms;
cl_int ret;

/* Zero out the buffer */
void MD5_init_once(md5_calc_t *calc)
{
	/* Zero out the struct */
	memset(calc, 0, sizeof(md5_calc_t));

	/* Load the source code containing the kernel*/
	FILE *fp = fopen("md5_cl.cl", "r");
	if (!fp) {
		fprintf(stderr, "Failed to load kernel.\n");
		exit(1);
	}

	char *source_str = (char*)malloc(MAX_SOURCE_SIZE);
	size_t source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
	fclose(fp);

	/* Get Platform and Device Info */
	ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
	assert(ret == CL_SUCCESS);

	ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &ret_num_devices);
	assert(ret == CL_SUCCESS);

	/* Create OpenCL context */
	context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
	assert(ret == CL_SUCCESS);

	/* Create Command Queue */
	command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
	assert(ret == CL_SUCCESS);

	/* Create Memory Buffer for key */
	memobj1 = clCreateBuffer(context, CL_MEM_READ_ONLY, 256, NULL, &ret);
	assert(ret == CL_SUCCESS);

	/* Create Memory Buffer for hash */
	memobj2 = clCreateBuffer(context, CL_MEM_WRITE_ONLY, 256, NULL, &ret);
	assert(ret == CL_SUCCESS);

	/* Create Kernel Program from the source */
	program = clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);
	assert(ret == CL_SUCCESS);

	/* Build Kernel Program */
	ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
	assert(ret == CL_SUCCESS);

	/* Create OpenCL Kernel */
	kernel = clCreateKernel(program, "MD5_hash", &ret);
	assert(ret == CL_SUCCESS);

	/* Set OpenCL Kernel Parameters */
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&memobj1);
	assert(ret == CL_SUCCESS);

	ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&memobj2);
	assert(ret == CL_SUCCESS);
}

/* Initialize for this size */
void MD5_init(md5_calc_t *calc, m128i_t *suffix, unsigned long size)
{
	calc->size_i = size / 4, calc->size_j = size % 4;
	calc->iv = 0x80 << (8 * calc->size_j);

	/* Copy suffix */
	for (int i = 0; i < 3; i++)
		calc->suffix[i] = suffix->i[i];

	/* Fill bytes 57-60 and add IV */
	calc->suffix[13] = (size << 3);
	if (calc->size_i > 0)
		calc->suffix[calc->size_i - 1] |= calc->iv;
}

/* Do 12 MD5 calculations */
void MD5_quad(md5_calc_t *calc, rainbow_t *rainbow, unsigned long size)
{
	size_t global = ENTRY_SIZE;

	/* For each of our three steps */
	for (int step = 0; step < STEP_SIZE; step++) {
		rainbow_t *r = &rainbow[step];

		/* For each of our four entries */
		for (int i = 0; i < ENTRY_SIZE; i++) {
			/* Copy prefix bytes */
			calc->prefixes[i] = r->prefixes.i[i];
			if (calc->size_i == 0)
				calc->prefixes[i] |= calc->iv;
		}

		clEnqueueWriteBuffer(command_queue, memobj1, CL_TRUE,
				     0, sizeof(md5_calc_t), calc, 0, NULL, NULL);
		clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global, NULL, 0, NULL, NULL);
		clEnqueueReadBuffer(command_queue, memobj2, CL_TRUE,
				    0, sizeof(md5_binary_t) * ENTRY_SIZE, r, 0, NULL, NULL);
	}
}
