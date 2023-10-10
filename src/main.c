#include <raylib.h>

enum { RADIUS = 4 };

void dot(int x, int y) {
    DrawCircle(x, y, RADIUS, BLACK);
}

int main() {
    SetTraceLogLevel(LOG_NONE);
    InitWindow(1000, 1000, "Draw");
    SetTargetFPS(144);

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
                int nX = GetMouseX();
                int nY = GetMouseY();

                if (!IsCursorHidden())
                    HideCursor();
            } else if (IsCursorHidden()) {
                ShowCursor();
            }
        }
        EndDrawing();
    }

    CloseWindow();
}

