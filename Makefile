# Makefile
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -I./Resources/raylib-master/src -I./Resources/raylib-master/src/external/glfw/include
LDFLAGS = -L./Resources/raylib-master/src
LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

program: VPN.c
	$(CC) $(CFLAGS) -o VPN VPN.c $(LDFLAGS) $(LIBS)

clean:
	rm -f VPN

.PHONY: clean