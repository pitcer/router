/**
 * Piotr Dobiech 316625
 */

#include "config.h"
#include "ip_utils.h"
#include "utils.h"
#include "vector.h"
#include <arpa/inet.h>
#include <errno.h>
#include <inttypes.h>
#include <malloc.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static inline size_t parse_input_distance_vector(VectorCell** vector_pointer) {
    uint32_t interfaces;
    scanf("%" SCNu32, &interfaces);

    VectorCell* vector = malloc(interfaces * sizeof(VectorCell));

    for (size_t index = 0; index < interfaces; index++) {
        VectorCell* cell = &vector[index];
        char address[STRING_ADDRESS_BUFFER_LENGTH];
        // TODO: scanf does not behave well with %s, because it does not know its length!
        scanf("%s/%" SCNu8 " distance %" SCNu32, address, &cell->mask_length, &cell->distance);
        try_string_address_to_binary(address, &cell->ip_address);
    }
    print_as_bytes(vector, 2 * sizeof(vector));
    *vector_pointer = vector;
    return interfaces;
}

static inline void send_distance_vector(const int socket_fd, struct sockaddr_in* receiver) {
    const char* message = "Hello server!";
    const size_t message_length = strlen(message);
    try_send_to(socket_fd, message, message_length, receiver);
}

static inline void receive_distance_vector(const int socket_fd) {
    struct sockaddr_in sender;
    uint8_t buffer[IP_MAXPACKET + 1];
    const ssize_t received_length = try_receive_from(socket_fd, buffer, IP_MAXPACKET, &sender);

    char sender_ip_string[STRING_ADDRESS_BUFFER_LENGTH];
    inet_ntop(AF_INET, &(sender.sin_addr), sender_ip_string, sizeof(sender_ip_string));
    println("Received UDP packet from IP address: %s, port: %d", sender_ip_string,
        ntohs(sender.sin_port));

    buffer[received_length] = 0;
    println("%ld-byte message: +%s+", received_length, buffer);
}

static inline void receive_or_send_distance_vector(const int socket_fd) {
    struct timeval select_time = { .tv_sec = TURN_TIME_SECONDS, .tv_usec = TURN_TIME_MICROS };

    while (1) {
        fd_set select_descriptors;
        FD_ZERO(&select_descriptors);
        FD_SET(socket_fd, &select_descriptors);

        const int ready = select(socket_fd + 1, &select_descriptors, NULL, NULL, &select_time);

        if (ready < 0) {
            eprintln("select error: %s", strerror(errno));
            exit(EXIT_FAILURE);
        } else if (ready == 0) {
            struct sockaddr_in receiver;
            try_create_socket_address("0.0.0.0", SERVER_PORT, &receiver);
            send_distance_vector(socket_fd, &receiver);

            select_time.tv_sec = TURN_TIME_SECONDS;
            select_time.tv_usec = TURN_TIME_MICROS;
        } else { // ready > 0
            receive_distance_vector(socket_fd);
        }
    }
}

int main(void) {
    VectorCell* vector;
    // print_as_bytes(vector, 2 * sizeof(vector));
    size_t vector_length = parse_input_distance_vector(&vector);
    print_as_bytes(vector, 2 * sizeof(vector));
    print_vector(vector, vector_length);

    const int socket_fd = try_create_socket();

    struct sockaddr_in server_address;
    try_create_socket_address("0.0.0.0", SERVER_PORT, &server_address);
    try_bind_socket(socket_fd, &server_address);

    receive_or_send_distance_vector(socket_fd);

    close(socket_fd);
    return EXIT_SUCCESS;
}
