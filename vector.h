/**
 * Piotr Dobiech 316625
 */

#pragma once

#include "adjacent_networks.h"
#include <inttypes.h>
#include <netinet/in.h>
#include <stdbool.h>

#define INFINITY_DISTANCE __UINT32_MAX__

#define CONNECTED_DIRECTLY 0
#define CONNECTED_VIA 1

typedef struct {
    uint32_t network_address;
    uint32_t distance;
    uint32_t indirect_address;
    uint8_t mask_length;
    uint8_t connection_type;
    // For how many turns that cell has distance set to infinity
    uint8_t unreachable_turns;
} VectorCell;

typedef struct {
    VectorCell* cells;
    size_t length;
} Vector;

void create_from_adjacent_networks(const AdjacentNetworks* networks, Vector* vector);

void create_cell_from_network(const AdjacentNetwork* network, VectorCell* cell);

VectorCell* find_cell(Vector* vector, const uint32_t network_address);

void add_cell(Vector* vector, VectorCell* cell);

void remove_cell(Vector* vector, VectorCell* cell);

void set_cells_unreachable_by_sender(Vector* vector, const uint32_t sender);

void remove_timeouted_cells_by_sender(Vector* vector, const uint32_t sender);

void handle_unreachable_vector_cells(Vector* vector);

void set_cells_unreachable(Vector* vector, const uint32_t network_address);

void set_cells_reachable(Vector* vector, const uint32_t distance, const uint32_t network_address);

void print_vector_cell(const VectorCell* cell);

void print_vector(const Vector* vector);

void deallocate_vector(Vector* vector);

typedef struct __attribute__((__packed__)) {
    in_addr_t network_address; // network byte order
    uint8_t mask_length;
    uint32_t distance; // network byte order
} VectorCellDatagram;

void wrap_vector_cell_datagram(const VectorCell* cell, VectorCellDatagram* datagram);

void unwrap_vector_cell_datagram(
    const VectorCellDatagram* datagram, const struct sockaddr_in* sender, VectorCell* cell);
