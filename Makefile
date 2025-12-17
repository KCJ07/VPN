# Makefile
# CC = gcc
# CFLAGS = -Wall -Wextra -std=c11 -I./Resources/raylib-master/src -I./Resources/raylib-master/src/external/glfw/include
# LDFLAGS = -L./Resources/raylib-master/src
# LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

# program: VPN.c
# 	$(CC) $(CFLAGS) -o VPN VPN.c $(LDFLAGS) $(LIBS)

# clean:
# 	rm -f VPN

# .PHONY: clean

# VPN Project Makefile
# Compile: make
# Clean: make clean
# Run: make run

# ------------------------- IGNORE ABOVE
CC = gcc
CFLAGS = -Wall -Wextra -pthread
TARGET = VPN

all: $(TARGET)

$(TARGET): VPNclientAndServer.c VPNencryption.c VPNtun.c VPNpacketqueue.c
	$(CC) $(CFLAGS) -o $(TARGET) VPNclientAndServer.c

clean:
	rm -f $(TARGET)

run: $(TARGET)
	sudo ./$(TARGET)

rebuild: clean all

.PHONY: all clean run rebuild