/**
 * Piotr Dobiech 316625
 */

#include "neighbours.h"

#include "config.h"
#include "ip_utils.h"
#include "utils.h"
#include <malloc.h>

void initialize_neighbours(Neighbours* neighbours) {
    neighbours->neighbours = NULL;
    neighbours->length = 0;
}

void create_neighbour(const uint32_t address, Neighbour* neighbour) {
    neighbour->address = address;
    neighbour->unreached_turns = 0;
    neighbour->reached_in_turn = false;
    neighbour->timeouted = false;
}

Neighbour* add_neighbour(Neighbours* neighbours, Neighbour* neighbour) {
    neighbours->length++;
    neighbours->neighbours
        = realloc(neighbours->neighbours, neighbours->length * sizeof(Neighbour));
    neighbours->neighbours[neighbours->length - 1] = *neighbour;
    return &neighbours->neighbours[neighbours->length - 1];
}

void remove_neighbour(Neighbours* neighbours, Neighbour* neighbour) {
    neighbours->length--;
    if (neighbours->length > 0) {
        *neighbour = neighbours->neighbours[neighbours->length];
    }
    neighbours->neighbours
        = realloc(neighbours->neighbours, neighbours->length * sizeof(Neighbour));
}

Neighbour* find_neighbour(Neighbours* neighbours, const uint32_t address) {
    for (size_t index = 0; index < neighbours->length; index++) {
        Neighbour* neighbour = &neighbours->neighbours[index];
        if (neighbour->address == address) {
            return neighbour;
        }
    }
    return NULL;
}

Neighbour* add_neighbour_if_absent(Neighbours* neighbours, const uint32_t address) {
    Neighbour* neighbour = find_neighbour(neighbours, address);
    if (neighbour == NULL) {
        Neighbour neighbour;
        create_neighbour(address, &neighbour);
        return add_neighbour(neighbours, &neighbour);
    } else {
        return neighbour;
    }
}

bool update_neighbour_unreached_turns(Neighbour* neighbour) {
    if (neighbour->timeouted) {
        return true;
    }

    bool reached = neighbour->reached_in_turn;
    neighbour->reached_in_turn = false;

    if (reached) {
        neighbour->unreached_turns = 0;
        return true;
    }

    neighbour->unreached_turns++;
    if (neighbour->unreached_turns == UNREACHED_TURNS_TO_TIMEOUT) {
        neighbour->unreached_turns = 0;
        return false;
    }

    return true;
}

void deallocate_neighbours(Neighbours* neighbours) { free(neighbours->neighbours); }
