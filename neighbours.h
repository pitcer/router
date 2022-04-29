/**
 * Piotr Dobiech 316625
 */

#pragma once

#include <arpa/inet.h>
#include <stdbool.h>

typedef struct {
    uint32_t address;
    uint8_t unreached_turns;
    bool reached_in_turn;
    bool timeouted;
} Neighbour;

typedef struct {
    Neighbour* neighbours;
    size_t length;
} Neighbours;

void initialize_neighbours(Neighbours* neighbours);

void create_neighbour(const uint32_t address, Neighbour* neighbour);

Neighbour* add_neighbour(Neighbours* neighbours, Neighbour* neighbour);

void remove_neighbour(Neighbours* neighbours, Neighbour* neighbour);

Neighbour* find_neighbour(Neighbours* neighbours, const uint32_t address);

Neighbour* add_neighbour_if_absent(Neighbours* neighbours, const uint32_t address);

/**
 * Returns false if network timed out
 */
bool update_neighbour_unreached_turns(Neighbour* neighbour);

void deallocate_neighbours(Neighbours* neighbours);
