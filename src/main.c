#include "raylib.h"
#include <stdio.h>

enum { RADIUS = 4 };

void dot(int x, int y) {
    DrawCircle(x, y, RADIUS, BLACK);
}

void erase(int x, int y) {
    DrawCircle(x, y, RADIUS * 2 , RAYWHITE);
}

bool which_buffer = false;

void swap() {
    SwapScreenBuffer();
}

void clear() {
    /* ClearBackground(RAYWHITE); */
    ClearBackground(RAYWHITE);
    swap();
}

int main() {
    InitWindow(1000, 1000, "Draw");
    int please_swap = 0;
    double previousTime = GetTime();
    double currentTime = 0.0;
    double updateDrawTime = 0.0;
    int targetFPS = 576;

    while(!WindowShouldClose()) {
        PollInputEvents();

        if (IsKeyPressed(KEY_UP))
            targetFPS *= 2;
        else if (IsKeyPressed(KEY_DOWN))
            targetFPS /= 2;
        
        if (targetFPS < 9) targetFPS = 9;

        if (IsKeyPressed(KEY_E)) {
            clear();
        }

        if (IsKeyPressed(KEY_S)) {
            swap();
        }

        BeginDrawing();
        {
            if (IsKeyDown(KEY_SPACE) || IsMouseButtonDown(0) || IsMouseButtonDown(4)) {
                please_swap++;
                dot(GetMouseX(), GetMouseY());
            } else if (IsKeyDown(KEY_C) || IsMouseButtonDown(1)) {
                please_swap++;
                erase(GetMouseX(), GetMouseY());
            }
            
            DrawRectangle(10, 10, 100, 20, RAYWHITE);
            DrawText(TextFormat("FPS: %d", targetFPS), 10, 10, 20, BLACK);
        }
        EndDrawing();

        if (please_swap % (targetFPS / 144) == 0) {
            please_swap = 0;
            swap();
        }

        currentTime = GetTime();
        updateDrawTime = currentTime - previousTime;

        if (targetFPS > 0) {
            float waitTime = (1.0f/(float)targetFPS) - updateDrawTime;
            if (waitTime > 0.0) {
                WaitTime((float)waitTime);
                currentTime = GetTime();
            }
        }

        previousTime = currentTime;
    }

    CloseWindow();
}
