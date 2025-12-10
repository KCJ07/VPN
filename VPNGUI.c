/// Kevin Johanson CSC 213 Final Project

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "Resources/raylib-master/src/raylib.h" 
#define RAYGUI_IMPLEMENTATION
#include "Resources/raygui-master/src/raygui.h"

#define PORT 5000
#define BUFFER_SIZE 1024
#define SERVER_IP "127.0.0.1"  // localhost



// starts the UDP with the server given
void startUDP(int server) {

    if (server == 0) {

    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    bzero(&server_addr, sizeof(server_addr));
    char buffer[BUFFER_SIZE];

    // Create UDP socket with IPV4
    // code taken and modified from "https://www.geeksforgeeks.org/computer-networks/udp-client-server-using-connect-c-implementation/"
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);
    servaddr.sin_family = AF_INET; 

    // converts address into binary
    printf("Client connected to server %s:%d\n", SERVER_IP, PORT);

    printf("UDP Server listening on port %d\n", PORT);
    }

}

// starts the connection depending on if its UDP or TCP 
void startConnection(int server, int connectionType) {
    if (connectionType == 0) {
        startUDP(server); 
    } else {
        printf("not IMPLEMENTED");
    }

}




int main(void)
{
    int screenWidth = 600;
    int screenHeight = 400;
    
    InitWindow(screenWidth, screenHeight, "VPN Connection Manager");
    SetTargetFPS(60);
    
    // Connection type
    int connectionType = 0;  // 0: UDP, 1: TCP
    int server = 0;  // 0 is server 1, 1 is server 2, 2 is server 3
    bool connected = false;
    bool dropdownOpen = false;

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        // Title
        DrawText("VPN CONNECTION MANAGER", 150, 30, 28, DARKBLUE);
        
        // Connection type section - Custom radio buttons
        DrawText("Connection Type:", 50, 100, 22, DARKGRAY);
        
        // Custom radio button for UDP
        Rectangle UDPRect = {250, 100, 20, 20};
        DrawRectangleLines(UDPRect.x, UDPRect.y, UDPRect.width, UDPRect.height, DARKGRAY);
        if (connectionType == 0) {
            DrawCircle(UDPRect.x + 10, UDPRect.y + 10, 6, DARKBLUE);
        }
        if (CheckCollisionPointRec(GetMousePosition(), UDPRect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            connectionType = 0;
        }
        DrawText("UDP", 280, 100, 20, DARKGRAY);
        
        // Custom radio button for TCP
        Rectangle TCPRect = {250, 130, 20, 20};
        DrawRectangleLines(TCPRect.x, TCPRect.y, TCPRect.width, TCPRect.height, DARKGRAY);
        if (connectionType == 1) {
            DrawCircle(TCPRect.x + 10, TCPRect.y + 10, 6, DARKBLUE);
        }
        if (CheckCollisionPointRec(GetMousePosition(), TCPRect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            connectionType = 1;
        }
        // Status display
        if (connected)
        {
            DrawRectangle(50, 310, 500, 60, Fade(GREEN, 0.2f));
            DrawText("STATUS: CONNECTED", 180, 325, 25, GREEN);
            
            const char* connectionTypes[] = {"UDP", "TCP"};
            const char* servers[] = {"TEST1", "TEST2", "TEST3"};

            // start the connections to the server selected 
            startConnection(server, connectionType);

            DrawText(TextFormat("Connected via %s to %s", connectionTypes[connectionType], servers[server]), 
                    60, 360, 18, DARKGRAY);
        }
        else
        {
            DrawRectangle(50, 310, 500, 60, Fade(RED, 0.2f));
            DrawText("STATUS: DISCONNECTED", 160, 325, 25, RED);
        }
        
        EndDrawing();
        // Connection buttons
        if (GuiButton((Rectangle){ 50, 240, 240, 50 }, "CONNECT")) 
        {
            connected = true;

        }
        
        if (GuiButton((Rectangle){ 310, 240, 240, 50 }, "DISCONNECT")) 
        {
            connected = false;
        }
                DrawText("TCP", 280, 130, 20, DARKGRAY);
        
        GuiSetStyle(DEFAULT, TEXT_SIZE, 27);
        // Server selection section
        DrawText("Select Server:", 50, 170, 22, DARKGRAY);
        if (GuiDropdownBox((Rectangle){ 250, 170, 300, 35 }, 
                          "TEST1", 
                          &server, dropdownOpen))
        {
            dropdownOpen = !dropdownOpen;
        }
        

    }
    
    CloseWindow();
    return 0;
}