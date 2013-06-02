#include "udp_socket.h"

int udp_socket_init(udp_socket_t *s, const char *address, const char *port)
{
	int retval = 0;
	struct addrinfo hints;
	struct addrinfo *p, *result;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	/* Make sure to set the socked descriptor to an invalid state */
	s->fd = -1;

	if (address == NULL || port == NULL) {
		errno = EDESTADDRREQ;
		return -1;
	}

	if ((retval = getaddrinfo(address, port, &hints, &result)) != 0)
		return retval;

	/* Loop through all the results and make a socket */
	for (p = result; p; p = p->ai_next) {
		s->fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (s->fd == -1)
			continue;

		if (connect(s->fd, p->ai_addr, p->ai_addrlen) != -1)
			break;

		close(s->fd);
	}

	if (!p)
		retval = -1;

	freeaddrinfo(result);

	return retval;
}

ssize_t udp_socket_send(udp_socket_t *s, const char *message, size_t length)
{
	if (!udp_socket_valid(s)) {
		errno = ENOTCONN;
		return -1;
	}

	return write(s->fd, message, length);
}

void udp_socket_destroy(udp_socket_t *s)
{
	close(s->fd);
	s->fd = -1;
}
