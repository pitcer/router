/**
 * Piotr Dobiech 316625
 */

#pragma once

#include "config.h"
#include <netinet/ip.h>
#include <stdbool.h>

uint32_t get_broadcast_address(const uint32_t address, const uint8_t mask_length);

uint32_t get_network_address(const uint32_t address, const uint8_t mask_length);

bool is_in_network(const uint32_t address, const uint32_t network, const uint8_t mask_length);

void string_address_to_binary(const char* string_address, void* buffer);

void binary_address_to_string(
    const void* address, char string_address[STRING_ADDRESS_BUFFER_LENGTH]);

void create_socket_address_from_binary(
    const uint32_t address, const uint16_t port, struct sockaddr_in* socket_address);

void create_socket_address(
    const char* address, const uint16_t port, struct sockaddr_in* socket_address);

int create_socket();

void enable_broadcast(const int socket_fd);

void bind_socket(const int socket_fd, const struct sockaddr_in* address);

ssize_t try_send_to(const int socket_fd, const void* buffer, const size_t buffer_length,
    const struct sockaddr_in* receiver);

void send_to(const int socket_fd, const void* buffer, const size_t buffer_length,
    const struct sockaddr_in* receiver);

ssize_t receive_from(
    const int socket_fd, void* sent_buffer, const size_t buffer_length, struct sockaddr_in* sender);

void print_socket_address(const struct sockaddr_in* socket_address);
