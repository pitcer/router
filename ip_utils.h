/**
 * Piotr Dobiech 316625
 */

#pragma once

#include <netinet/ip.h>

void create_socket_address(
    const char* address, const uint16_t port, struct sockaddr_in* socket_address);

int try_create_socket();

void try_bind_socket(const int socket_fd, const struct sockaddr_in* address);

void try_send_to(const int socket_fd, const void* buffer, const size_t buffer_length,
    const struct sockaddr_in* receiver);

ssize_t try_receive_from(const int socket_fd, void* sent_buffer, const size_t buffer_length,
    struct sockaddr_in* sender);
