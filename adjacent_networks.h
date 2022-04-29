/**
 * Piotr Dobiech 316625
 */

#pragma once

#include <arpa/inet.h>
#include <stdbool.h>

typedef struct {
    uint32_t interface_address;
    uint32_t distance;
    uint8_t mask_length;
    uint8_t unreached_turns;
    bool reached_in_turn;
    bool timed_out;
} AdjacentNetwork;

typedef struct {
    AdjacentNetwork* networks;
    size_t length;
} AdjacentNetworks;

void get_adjacent_network_broadcast_socket_address(
    const AdjacentNetwork* network, struct sockaddr_in* broadcast_address);

uint32_t get_adjacent_network_network_address(const AdjacentNetwork* network);

bool is_in_adjacent_network(const AdjacentNetwork* network, const uint32_t address);

AdjacentNetwork* get_network(AdjacentNetworks* networks, const uint32_t address);

AdjacentNetwork* find_adjacent_network(AdjacentNetworks* networks, const uint32_t address);

/**
 * Returns false if network timed out
 */
bool update_unreached_turns(AdjacentNetwork* network);

void deallocate_adjacent_networks(AdjacentNetworks* networks);
