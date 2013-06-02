CC := gcc
CFLAGS := -std=gnu99 -Os
OBJS := charset.o charset_lowercase.o charset_numeric.o cpus.o
TARGET := main

all: $(TARGET)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(TARGET): main.c $(OBJS)
	$(CC) $< $(CFLAGS) $(OBJS) -o $@

.PHONY: all clean
clean:
	rm -f $(OBJS) $(TARGET)
