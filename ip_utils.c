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

uint32_t get_broadcast_address(const uint32_t address, const uint8_t mask_length) {
    const uint32_t ones = ~0u;
    const uint32_t mask = ones >> mask_length;
    return address | mask;
}

uint32_t get_network_address(const uint32_t address, const uint8_t mask_length) {
    const uint32_t ones = ~0u;
    const uint32_t mask = ones >> mask_length;
    return address & ~mask;
}

bool is_in_network(const uint32_t address, const uint32_t network, const uint8_t mask_length) {
    const uint32_t address_prefix = get_network_address(address, mask_length);
    return address_prefix == network;
}

void string_address_to_binary(const char* string_address, void* buffer) {
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

void binary_address_to_string(
    const void* address, char string_address[STRING_ADDRESS_BUFFER_LENGTH]) {
    const char* result = inet_ntop(AF_INET, address, string_address, STRING_ADDRESS_BUFFER_LENGTH);
    if (result == NULL) {
        eprintln("inet_pton error: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

/**
 * address and port should be in the host order.
 */
void create_socket_address_from_binary(
    const uint32_t address, const uint16_t port, struct sockaddr_in* socket_address) {
    memset(socket_address, 0, sizeof(*socket_address));
    socket_address->sin_family = AF_INET;
    socket_address->sin_port = htons(port);
    socket_address->sin_addr.s_addr = htonl(address);
}

void create_socket_address(
    const char* address, const uint16_t port, struct sockaddr_in* socket_address) {
    uint32_t binary_address;
    string_address_to_binary(address, &binary_address);
    uint32_t binary_host_address = ntohl(binary_address);
    create_socket_address_from_binary(binary_host_address, port, socket_address);
}

int create_socket() {
    const int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        eprintln("socket error: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return socket_fd;
}

void enable_broadcast(const int socket_fd) {
    int broadcast_permission = 1;
    int result = setsockopt(socket_fd, SOL_SOCKET, SO_BROADCAST, (void*)&broadcast_permission,
        sizeof(broadcast_permission));
    if (result == -1) {
        eprintln("setsockopt error: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void bind_socket(const int socket_fd, const struct sockaddr_in* address) {
    const int result = bind(socket_fd, (struct sockaddr*)address, sizeof(*address));
    if (result < 0) {
        eprintln("bind error: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

ssize_t try_send_to(const int socket_fd, const void* buffer, const size_t buffer_length,
    const struct sockaddr_in* receiver) {

    return sendto(
        socket_fd, buffer, buffer_length, 0, (struct sockaddr*)receiver, sizeof(*receiver));
}

void send_to(const int socket_fd, const void* buffer, const size_t buffer_length,
    const struct sockaddr_in* receiver) {

    const ssize_t result = try_send_to(socket_fd, buffer, buffer_length, receiver);
    if (result != (ssize_t)buffer_length) {
        eprintln("sendto error: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

ssize_t receive_from(const int socket_fd, void* sent_buffer, const size_t buffer_length,
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

void print_socket_address(const struct sockaddr_in* socket_address) {
    char address[STRING_ADDRESS_BUFFER_LENGTH];
    binary_address_to_string(&(socket_address->sin_addr), address);
    uint16_t port = ntohs(socket_address->sin_port);
    println("%s:%" PRIu16, address, port);
}
