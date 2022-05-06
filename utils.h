/**
 * Piotr Dobiech 316625
 */

#pragma once

#include <stdio.h>

#define println(format, ...) printf(format "\n" __VA_OPT__(, ) __VA_ARGS__)

#define eprintln(format, ...) fprintf(stderr, format "\n" __VA_OPT__(, ) __VA_ARGS__)

// #define DEBUG
#ifdef DEBUG
#define debug(...) println(__VA_ARGS__)
#else
#define debug(...)
#endif

void print_as_bytes(unsigned char* buff, ssize_t length);
