CFLAGS := -dead_strip -Os -msse4.2 -std=gnu99
CC := gcc
OBJS := cpus.o md5_sse.o md5_string.o
TARGET := bin/md5_crack

all: $(TARGET)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(TARGET): $(OBJS) md5_crack.c
	$(CC) md5_crack.c $(CFLAGS) $(OBJS) -o bin/md5_crack

.PHONY: all clean
clean:
	rm -f *.o bin/*
