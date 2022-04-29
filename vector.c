/**
 * Piotr Dobiech 316625
 */

#include "vector.h"

#include "config.h"
#include "ip_utils.h"
#include "utils.h"
#include <arpa/inet.h>
#include <malloc.h>
#include <stdlib.h>

void create_from_adjacent_networks(const AdjacentNetworks* networks, Vector* vector) {
    vector->length = networks->length;
    vector->cells = malloc(vector->length * sizeof(VectorCell));

    for (size_t index = 0; index < networks->length; index++) {
        const AdjacentNetwork network = networks->networks[index];
        VectorCell* cell = &vector->cells[index];
        cell->network_address = get_network_address(network.interface_address, network.mask_length);
        cell->mask_length = network.mask_length;
        cell->distance = network.distance;
        cell->connection_type = CONNECTED_DIRECTLY;
        cell->indirect_address = 0;
        cell->disabled = false;
    }
}

VectorCell* find_cell(Vector* vector, const uint32_t network_address) {
    for (size_t index = 0; index < vector->length; index++) {
        VectorCell* cell = &vector->cells[index];
        if (cell->network_address == network_address) {
            return cell;
        }
    }
    return NULL;
}

void add_cell(Vector* vector, VectorCell* cell) {
    vector->length++;
    vector->cells = realloc(vector->cells, vector->length * sizeof(VectorCell));
    vector->cells[vector->length - 1] = *cell;
}

void remove_cell(Vector* vector, VectorCell* cell) {
    vector->length--;
    if (vector->length > 0) {
        *cell = vector->cells[vector->length];
    }
    vector->cells = realloc(vector->cells, vector->length * sizeof(VectorCell));
}

void set_cells_unreachable_by_sender(Vector* vector, const uint32_t sender) {

    for (size_t index = 0; index < vector->length; index++) {
        VectorCell* cell = &vector->cells[index];
        if (cell->connection_type == CONNECTED_VIA && cell->indirect_address == sender) {
            cell->distance = INFINITY_DISTANCE;
        }
    }
}

void remove_timeouted_cells_by_sender(Vector* vector, const uint32_t sender) {

    for (size_t index = 0; index < vector->length; index++) {
        VectorCell* cell = &vector->cells[index];
        if (cell->connection_type == CONNECTED_VIA && cell->indirect_address == sender) {
            remove_cell(vector, cell);
            index--;
        }
    }
}

void set_cells_unreachable(
    Vector* vector, const uint32_t network_address, const uint32_t mask_length) {

    for (size_t index = 0; index < vector->length; index++) {
        VectorCell* cell = &vector->cells[index];
        if (cell->network_address == network_address) {
            cell->distance = INFINITY_DISTANCE;
        }
    }
}

void set_cells_enabled(
    Vector* vector, const uint32_t network_address, const uint32_t mask_length) {

    for (size_t index = 0; index < vector->length; index++) {
        VectorCell* cell = &vector->cells[index];
        if (cell->network_address == network_address) {
            cell->disabled = false;
        }
    }
}

void set_cells_disabled(
    Vector* vector, const uint32_t network_address, const uint32_t mask_length) {

    for (size_t index = 0; index < vector->length; index++) {
        VectorCell* cell = &vector->cells[index];
        if (cell->network_address == network_address) {
            cell->disabled = true;
        }
    }
}

void print_vector_cell(const VectorCell* cell) {
    char address[STRING_ADDRESS_BUFFER_LENGTH];
    const in_addr_t network_address = htonl(cell->network_address);
    binary_address_to_string(&network_address, address);
    printf("%s/%" PRIu8, address, cell->mask_length);

    if (cell->distance == INFINITY_DISTANCE) {
        printf(" unreachable");
    } else {
        printf(" distance %" PRIu32, cell->distance);
    }

    if (cell->connection_type == CONNECTED_DIRECTLY) {
        printf(" connected directly");
    } else if (cell->connection_type == CONNECTED_VIA) {
        char indirect_address_string[STRING_ADDRESS_BUFFER_LENGTH];
        const in_addr_t indirect_address = htonl(cell->indirect_address);
        binary_address_to_string(&indirect_address, indirect_address_string);
        printf(" via %s", indirect_address_string);
    } else {
        eprintln("\nInvalid connection type");
        exit(EXIT_FAILURE);
    }

    println();
}

void print_vector(const Vector* vector) {
    for (size_t index = 0; index < vector->length; index++) {
        const VectorCell* cell = &vector->cells[index];
        print_vector_cell(cell);
    }
    println();
}

void deallocate_vector(Vector* vector) { free(vector->cells); }

void wrap_vector_cell_datagram(const VectorCell* cell, VectorCellDatagram* datagram) {
    datagram->network_address = htonl(cell->network_address);
    datagram->mask_length = cell->mask_length;
    datagram->distance = htonl(cell->distance);
}

void unwrap_vector_cell_datagram(
    const VectorCellDatagram* datagram, const struct sockaddr_in* sender, VectorCell* cell) {
    cell->network_address = ntohl(datagram->network_address);
    cell->mask_length = datagram->mask_length;
    cell->distance = ntohl(datagram->distance);
    cell->connection_type = CONNECTED_VIA;
    cell->indirect_address = ntohl(sender->sin_addr.s_addr);
}
