# Piotr Dobiech 316625

CC = gcc
CFLAGS = -std=gnu17 -Wall -Wextra -g

BINARY_NAME := router
SRCS := utils.c ip_utils.c vector.c adjacent_networks.c neighbours.c main.c
OBJS := $(SRCS:%.c=%.o)

$(BINARY_NAME): $(OBJS)
	$(CC) $(OBJS) -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm $(OBJS)

cleandist:
	rm $(OBJS) $(BINARY_NAME)
