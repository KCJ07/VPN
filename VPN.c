/// Kevin Johanson CSC 213 Final Project

#include "Resources/raylib/src/raylib.h" 
#define RAYGUI_IMPLEMENTATION
#include "Resources/raylib/raygui/src/raygui.h"

int main(void)
{
    int screenWidth = 600;
    int screenHeight = 400;
    
    InitWindow(screenWidth, screenHeight, "VPN Connection Manager");
    SetTargetFPS(60);
    
    // Connection type
    int connectionType = 0;  // 0: Wireless, 1: Wired
    int server = 0;
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
        
        // Custom radio button for Wireless
        Rectangle wirelessRect = {250, 100, 20, 20};
        DrawRectangleLines(wirelessRect.x, wirelessRect.y, wirelessRect.width, wirelessRect.height, DARKGRAY);
        if (connectionType == 0) {
            DrawCircle(wirelessRect.x + 10, wirelessRect.y + 10, 6, DARKBLUE);
        }
        if (CheckCollisionPointRec(GetMousePosition(), wirelessRect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            connectionType = 0;
        }
        DrawText("Wireless", 280, 100, 20, DARKGRAY);
        
        // Custom radio button for Wired
        Rectangle wiredRect = {250, 130, 20, 20};
        DrawRectangleLines(wiredRect.x, wiredRect.y, wiredRect.width, wiredRect.height, DARKGRAY);
        if (connectionType == 1) {
            DrawCircle(wiredRect.x + 10, wiredRect.y + 10, 6, DARKBLUE);
        }
        if (CheckCollisionPointRec(GetMousePosition(), wiredRect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            connectionType = 1;
        }
        DrawText("Wired", 280, 130, 20, DARKGRAY);
        
        // Server selection section
        DrawText("Select Server:", 50, 170, 22, DARKGRAY);
        if (GuiDropdownBox((Rectangle){ 250, 170, 300, 35 }, 
                          "US East Server;US West Server;Europe Server;Asia Server", 
                          &server, dropdownOpen))
        {
            dropdownOpen = !dropdownOpen;
        }
        
        // Connection buttons
        if (GuiButton((Rectangle){ 50, 240, 240, 50 }, "CONNECT")) 
        {
            connected = true;
        }
        
        if (GuiButton((Rectangle){ 310, 240, 240, 50 }, "DISCONNECT")) 
        {
            connected = false;
        }
        
        // Status display
        if (connected)
        {
            DrawRectangle(50, 310, 500, 60, Fade(GREEN, 0.2f));
            DrawText("STATUS: CONNECTED", 180, 325, 32, GREEN);
            
            const char* connectionTypes[] = {"Wireless", "Wired"};
            const char* servers[] = {"US East Server", "US West Server", "Europe Server", "Asia Server"};
            DrawText(TextFormat("Connected via %s to %s", connectionTypes[connectionType], servers[server]), 
                    60, 360, 18, DARKGRAY);
        }
        else
        {
            DrawRectangle(50, 310, 500, 60, Fade(RED, 0.2f));
            DrawText("STATUS: DISCONNECTED", 160, 325, 32, RED);
        }
        
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}