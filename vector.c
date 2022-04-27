/**
 * Piotr Dobiech 316625
 */

#include "vector.h"

#include "config.h"
#include "utils.h"
#include <arpa/inet.h>

// TODO: use vector cell struct
void fill_vector_cell_datagram(const uint32_t address, const uint8_t mask_length,
    const uint32_t distance, VectorCellDatagram* datagram) {
    datagram->ip_address = htonl(address);
    datagram->mask_length = mask_length;
    datagram->distance = htonl(distance);
}

void print_vector(const VectorCell* vector, const size_t vector_length) {
    print_as_bytes(vector, vector_length * sizeof(vector));

    for (size_t index = 0; index < vector_length; index++) {
        VectorCell cell = vector[index];
        char address[STRING_ADDRESS_BUFFER_LENGTH];
        inet_ntop(AF_INET, &cell.ip_address, address, sizeof(address));
        println("%s/%" PRIu8 " distance %" PRIu32, address, cell.mask_length, cell.distance);
    }
}
