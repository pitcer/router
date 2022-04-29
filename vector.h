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
    // We can use bit fields here, but because of padding, we can introduce up to 3 additional
    // bytes.
    uint8_t mask_length;
    uint8_t connection_type;
    bool disabled;
} VectorCell;

typedef struct {
    VectorCell* cells;
    size_t length;
} Vector;


void set_cells_enabled(
    Vector* vector, const uint32_t network_address, const uint32_t mask_length);

void set_cells_disabled(
    Vector* vector, const uint32_t network_address, const uint32_t mask_length);

void create_from_adjacent_networks(const AdjacentNetworks* networks, Vector* vector);

VectorCell* find_cell(Vector* vector, const uint32_t network_address);

void add_cell(Vector* vector, VectorCell* cell);

void set_cells_unreachable_by_sender(Vector* vector, const uint32_t sender);

void remove_timeouted_cells_by_sender(Vector* vector, const uint32_t sender);

void set_cells_unreachable(
    Vector* vector, const uint32_t network_address, const uint32_t mask_length);

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
