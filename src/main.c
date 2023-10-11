#include <raylib.h>

enum { RADIUS = 4 };

void dot(int x, int y) {
    DrawCircle(x, y, RADIUS, BLACK);
}

void erase(int x, int y) {
    DrawCircle(x, y, RADIUS * 2 , RAYWHITE);
}

int main() {
    SetTraceLogLevel(LOG_NONE);
    InitWindow(1000, 1000, "Draw");
    SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));

    BeginDrawing();
    ClearBackground(RAYWHITE);
    EndDrawing();

    while(!WindowShouldClose()) {
        BeginDrawing();
        {
            if (IsKeyDown(KEY_E)) {
                ClearBackground(RAYWHITE);
            }
                    
            if (IsKeyDown(KEY_X) || IsKeyDown(KEY_SPACE)) {
                dot(GetMouseX(), GetMouseY());

                if (!IsCursorHidden())
                    HideCursor();
            } else if (IsKeyDown(KEY_C)) {
                erase(GetMouseX(), GetMouseY());
            } else if (IsCursorHidden()) {
                ShowCursor();
            }

        }
        EndDrawing();
    }

    CloseWindow();
}

