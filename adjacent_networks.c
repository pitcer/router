/**
 * Piotr Dobiech 316625
 */

#include "adjacent_networks.h"

#include "config.h"
#include "ip_utils.h"
#include "utils.h"
#include <malloc.h>

void get_adjacent_network_broadcast_socket_address(
    const AdjacentNetwork* network, struct sockaddr_in* broadcast_address) {

    const uint32_t broadcast
        = get_broadcast_address(network->interface_address, network->mask_length);
    create_socket_address_from_binary(broadcast, SERVER_PORT, broadcast_address);
}

uint32_t get_adjacent_network_network_address(const AdjacentNetwork* network) {
    return get_network_address(network->interface_address, network->mask_length);
}

bool is_in_adjacent_network(const AdjacentNetwork* network, const uint32_t address) {
    return is_in_network(address, network->interface_address, network->mask_length);
}

AdjacentNetwork* find_adjacent_network(AdjacentNetworks* networks, const uint32_t address) {
    for (size_t index = 0; index < networks->length; index++) {
        AdjacentNetwork* network = &networks->networks[index];
        if (network->interface_address == address) {
            return network;
        }
    }
    return NULL;
}

AdjacentNetwork* get_network(AdjacentNetworks* networks, const uint32_t address) {
    for (size_t index = 0; index < networks->length; index++) {
        AdjacentNetwork* network = &networks->networks[index];
        const uint32_t interface_network
            = get_network_address(network->interface_address, network->mask_length);
        if (is_in_network(address, interface_network, network->mask_length)) {
            return network;
        }
    }
    return NULL;
}

void deallocate_adjacent_networks(AdjacentNetworks* networks) { free(networks->networks); }
