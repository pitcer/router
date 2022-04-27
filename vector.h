/**
 * Piotr Dobiech 316625
 */

#pragma once

#include <inttypes.h>
#include <netinet/in.h>

#define INFINITY_DISTANCE __UINT32_MAX__

typedef struct {
    in_addr_t ip_address;
    uint8_t mask_length;
    uint32_t distance;
} VectorCell;

typedef struct __attribute__((__packed__)) {
    in_addr_t ip_address;
    uint8_t mask_length;
    uint32_t distance;
} VectorCellDatagram;

// void create_vector_cell_datagram(VectorCellDatagram* datagram);

void print_vector(const VectorCell* vector, const size_t vector_length);
