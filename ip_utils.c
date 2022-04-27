/**
 * Piotr Dobiech 316625
 */

#include "ip_utils.h"

#include "utils.h"
#include <arpa/inet.h>
#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

// TODO: replace remaining inet_pton's to this helper
void try_string_address_to_binary(const char* string_address, void* buffer) {
    int result = inet_pton(AF_INET, string_address, buffer);
    if (result == 0) {
        eprintln("inet_pton error: given address does not contain a character string representing "
                 "a valid network address in the AF_INET address family.");
        exit(EXIT_FAILURE);
    }
    if (result < 0) {
        eprintln("inet_pton error: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

// TODO: add ^^^ but for inet_ntop

void try_create_socket_address(
    const char* address, const uint16_t port, struct sockaddr_in* socket_address) {
    memset(socket_address, 0, sizeof(*socket_address));
    socket_address->sin_family = AF_INET;
    socket_address->sin_port = htons(port);
    try_string_address_to_binary(address, &socket_address->sin_addr);
}

int try_create_socket() {
    const int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        eprintln("socket error: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return socket_fd;
}

void try_bind_socket(const int socket_fd, const struct sockaddr_in* address) {
    const int result = bind(socket_fd, (struct sockaddr*)address, sizeof(*address));
    if (result < 0) {
        eprintln("bind error: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void try_send_to(const int socket_fd, const void* buffer, const size_t buffer_length,
    const struct sockaddr_in* receiver) {

    const ssize_t result = sendto(
        socket_fd, buffer, buffer_length, 0, (struct sockaddr*)receiver, sizeof(*receiver));
    if (result != (ssize_t)buffer_length) {
        eprintln("sendto error: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

ssize_t try_receive_from(const int socket_fd, void* sent_buffer, const size_t buffer_length,
    struct sockaddr_in* sender) {

    socklen_t sender_length = sizeof(*sender);
    const ssize_t result = recvfrom(
        socket_fd, sent_buffer, buffer_length, 0, (struct sockaddr*)sender, &sender_length);
    if (result < 0) {
        eprintln("recvfrom error: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return result;
}
