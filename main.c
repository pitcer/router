/**
 * Piotr Dobiech 316625
 */

#include "adjacent_networks.h"
#include "config.h"
#include "ip_utils.h"
#include "neighbours.h"
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

static inline void parse_input_adjacent_networks(AdjacentNetworks* networks) {
    uint32_t networks_length;
    scanf("%" SCNu32, &networks_length);
    networks->length = networks_length;

    networks->networks = malloc(networks_length * sizeof(AdjacentNetwork));

    for (size_t index = 0; index < networks_length; index++) {
        AdjacentNetwork* network = &networks->networks[index];
        uint8_t address_bytes[4];
        scanf("%" SCNu8 ".%" SCNu8 ".%" SCNu8 ".%" SCNu8 "/%" SCNu8 " distance %" SCNu32,
            &address_bytes[3], &address_bytes[2], &address_bytes[1], &address_bytes[0],
            &network->mask_length, &network->distance);
        network->interface_address = *((uint32_t*)address_bytes);
        network->unreached_turns = 0;
        network->reached_in_turn = false;
        network->timed_out = false;
    }
}

static inline void send_vector(const int socket_fd, Vector* vector, AdjacentNetwork* network) {

    struct sockaddr_in receiver;
    get_adjacent_network_broadcast_socket_address(network, &receiver);

    for (size_t index = 0; index < vector->length; index++) {
        const VectorCell* cell = &vector->cells[index];
        if (cell->disabled) {
            continue;
        }
        VectorCellDatagram datagram;
        wrap_vector_cell_datagram(cell, &datagram);
        const ssize_t result = try_send_to(socket_fd, &datagram, sizeof(datagram), &receiver);
        if (result != (ssize_t)sizeof(datagram)) {
            debug("send to failed");
            const uint32_t network_address = get_adjacent_network_network_address(network);
            network->timed_out = true;
            network->unreached_turns = 0;
            set_cells_unreachable(vector, network_address, network->mask_length);
        }
    }
}

static inline void send_vector_to_networks(
    const int socket_fd, Vector* vector, AdjacentNetworks* networks) {

    for (size_t index = 0; index < networks->length; index++) {
        AdjacentNetwork* network = &networks->networks[index];
        send_vector(socket_fd, vector, network);
    }
}

static inline void update_vector_cell(VectorCell* vector_cell, const VectorCell* received_cell) {
    if (vector_cell->distance > received_cell->distance
        || (received_cell->connection_type == CONNECTED_VIA
            && received_cell->indirect_address == vector_cell->indirect_address
            && received_cell->distance == INFINITY_DISTANCE)) {
        *vector_cell = *received_cell;
    }
}

static inline void receive_vector_from_networks(
    const int socket_fd, Vector* vector, AdjacentNetworks* networks, Neighbours* neighbours) {

    struct sockaddr_in sender;
    uint8_t buffer[IP_MAXPACKET + 1];
    const ssize_t received_length = receive_from(socket_fd, buffer, IP_MAXPACKET, &sender);
    if (received_length != sizeof(VectorCellDatagram)) {
        debug("packet with invalid length received");
        return;
    }
    const VectorCellDatagram* datagram = (VectorCellDatagram*)buffer;
    VectorCell received_cell;
    unwrap_vector_cell_datagram(datagram, &sender, &received_cell);


    AdjacentNetwork* adjacent_network
        = find_adjacent_network(networks, received_cell.indirect_address);
    if (adjacent_network != NULL) {
        adjacent_network->reached_in_turn = true;
        // Received packet from ourself
        return;
    }

    Neighbour* neighbour = add_neighbour_if_absent(neighbours, received_cell.indirect_address);
    neighbour->reached_in_turn = true;

    AdjacentNetwork* network = get_network(networks, received_cell.indirect_address);
    if (network == NULL) {
        debug("packet from unknown sender");
        return;
    }
    const uint32_t network_address = get_adjacent_network_network_address(network);
    set_cells_enabled(vector, network_address, network->mask_length);
    network->reached_in_turn = true;
    received_cell.distance += network->distance;

    // println("vec:");
    // print_vector_cell(&received_cell);

    VectorCell* vector_cell = find_cell(vector, received_cell.network_address);
    if (vector_cell == NULL) {
        add_cell(vector, &received_cell);
    } else {
        update_vector_cell(vector_cell, &received_cell);
    }
}

static inline void handle_timed_out_networks(
    Vector* vector, AdjacentNetworks* networks, Neighbours* neighbours) {

    for (size_t index = 0; index < neighbours->length; index++) {
        Neighbour* neighbour = &neighbours->neighbours[index];

        if (neighbour->reached_in_turn) {
            neighbour->reached_in_turn = false;
            neighbour->timeouted = false;
            neighbour->unreached_turns = 0;
        } else {
            neighbour->unreached_turns++;
            if (neighbour->timeouted && neighbour->unreached_turns == TIMEOUTED_TURNS_TO_REMOVAL) {
                remove_timeouted_cells_by_sender(vector, neighbour->address);
                remove_neighbour(neighbours, neighbour);
                index--;
            } else if (!neighbour->timeouted
                && neighbour->unreached_turns == UNREACHED_TURNS_TO_TIMEOUT) {

                neighbour->unreached_turns = 0;
                neighbour->timeouted = true;
                set_cells_unreachable_by_sender(vector, neighbour->address);
            }
        }
    }

    for (size_t index = 0; index < networks->length; index++) {
        AdjacentNetwork* network = &networks->networks[index];

        if (network->reached_in_turn) {
            network->reached_in_turn = false;
            network->timed_out = false;
            network->unreached_turns = 0;
        } else {
            network->unreached_turns++;
            if (network->timed_out && network->unreached_turns == TIMEOUTED_TURNS_TO_REMOVAL) {
                remove_timeouted_cells_by_sender(vector, network->interface_address);
                // remove_network(networks, network);
                const uint32_t network_address = get_adjacent_network_network_address(network);
                set_cells_disabled(vector, network_address, network->mask_length);
                index--;
            } else if (!network->timed_out
                && network->unreached_turns == UNREACHED_TURNS_TO_TIMEOUT) {

                network->unreached_turns = 0;
                network->timed_out = true;
                const uint32_t network_address = get_adjacent_network_network_address(network);
                set_cells_unreachable(vector, network_address, network->mask_length);
            }
        }
        // bool result = update_unreached_turns(network);
        // if (!result) {
        //     const uint32_t network_address = get_adjacent_network_network_address(network);
        //     set_cells_unreachable(vector, network_address, network->mask_length);
        // }
    }
}

static inline void receive_or_send_distance_vector(
    const int socket_fd, Vector* vector, AdjacentNetworks* networks, Neighbours* neighbours) {

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
            handle_timed_out_networks(vector, networks, neighbours);
            send_vector_to_networks(socket_fd, vector, networks);

            print_vector(vector);

            select_time.tv_sec = TURN_TIME_SECONDS;
            select_time.tv_usec = TURN_TIME_MICROS;
        } else { // ready > 0
            receive_vector_from_networks(socket_fd, vector, networks, neighbours);
        }
    }
}

int main(void) {
    AdjacentNetworks networks;
    parse_input_adjacent_networks(&networks);

    Vector vector;
    create_from_adjacent_networks(&networks, &vector);
    print_vector(&vector); // TODO: remove

    Neighbours neighbours;
    initialize_neighbours(&neighbours);

    const int socket_fd = create_socket();
    enable_broadcast(socket_fd);

    struct sockaddr_in server_address;
    create_socket_address(SERVER_ADDRESS, SERVER_PORT, &server_address);
    bind_socket(socket_fd, &server_address);

    receive_or_send_distance_vector(socket_fd, &vector, &networks, &neighbours);

    deallocate_adjacent_networks(&networks);
    deallocate_vector(&vector);
    close(socket_fd);
    return EXIT_SUCCESS;
}
