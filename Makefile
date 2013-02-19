CFLAGS := -Os -msse4.2 -std=gnu99 -flax-vector-conversions
CLCFLAGS := $(CFLAGS) -I/usr/include/nvidia-current-updates/
LIBS := -lpthread -lm
CLLIBS = $(LIBS) -lOpenCL
CC := gcc
OBJS := cpus.o md5_sse.o md5_string.o
CL_OBJS := cpus.o md5_cl.o md5_string.o
TARGET := bin/md5_crack
TARGET_CL := bin/md5_crack_cl

all: $(TARGET) $(TARGET_CL)

%.o: %.c
	$(CC) -c $(CLCFLAGS) $< -o $@

$(TARGET): $(OBJS) md5_crack.c
	$(CC) md5_crack.c $(CFLAGS) $(OBJS) $(LIBS) -o bin/md5_crack

$(TARGET_CL): $(CL_OBJS) md5_crack.c
	$(CC) -DOPENCL md5_crack.c $(CLCFLAGS) $(CL_OBJS) $(CLLIBS) -o bin/md5_crack_cl

.PHONY: all clean
clean:
	rm -f *.o bin/*
