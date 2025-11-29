# Makefile
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -I./Resources/raylib/src -I./Resources/raylib/src/external/glfw/include
LDFLAGS = -L./Resources/raylib/src
LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

program: VPN.c
	$(CC) $(CFLAGS) -o VPN VPN.c $(LDFLAGS) $(LIBS)

clean:
	rm -f VPN

.PHONY: clean