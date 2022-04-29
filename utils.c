/**
 * Piotr Dobiech 316625
 */

#include "utils.h"

#include <inttypes.h>

void print_as_bytes(unsigned char* buff, ssize_t length) {
    for (ssize_t i = 0; i < length; i++, buff++) {
        printf("%.2x ", *buff);
    }
    println();
}
