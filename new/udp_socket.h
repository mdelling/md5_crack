#ifndef UDPSOCKET_H
#define UDPSOCKET_H

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include "common.h"

#define UDP_SOCKET_INIT() -1

typedef struct udp_socket {
	int fd;
} udp_socket_t;

int udp_socket_init(udp_socket_t *s,
		    const char *address,
		    const char *port) NONNULL(1,2,3);
ssize_t udp_socket_send(udp_socket_t *ss, const char *message, size_t length) NONNULL(1,2);

static inline int udp_socket_valid(const udp_socket_t *s)
{
	return s->fd != -1;
} NONNULL(1)

void udp_socket_destroy(udp_socket_t *s) NONNULL(1);

#endif
