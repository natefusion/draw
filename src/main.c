#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define WIDTH 1000
#define HEIGHT 1000

struct Strokes {
    int xy_size;
    int xy_capacity;
    int num_strokes;
    int strokes_capacity;
    int *stroke_offsets;
    int *stroke_sizes;
    int *x;
    int *y;
};

void add_point(struct Strokes *s, int x, int y) {
    if (s->xy_size >= 1 && s->x[s->xy_size - 1] == x && s->y[s->xy_size - 1] == y) return;
    
    s->xy_size += 1;
    s->stroke_sizes[s->num_strokes-1] += 1;

    if (s->xy_size >= s->xy_capacity) {
        s->xy_capacity = 1 + (int)roundf(1.618033989f * (float)s->xy_capacity);
        s->x = realloc(s->x, s->xy_capacity * sizeof(int));
        s->y = realloc(s->y, s->xy_capacity * sizeof(int));
    }
    
    s->x[s->xy_size - 1] = x;
    s->y[s->xy_size - 1] = y;
}

void add_stroke(struct Strokes *s) {
    s->num_strokes += 1;
    if (s->num_strokes >= s->strokes_capacity) {
        s->strokes_capacity = 1 + (int)roundf(1.618033989f * (float)s->strokes_capacity);
        s->stroke_offsets = realloc(s->stroke_offsets, s->strokes_capacity * sizeof(int));
        s->stroke_sizes = realloc(s->stroke_sizes, s->strokes_capacity * sizeof(int));
    }

    s->stroke_offsets[s->num_strokes-1] = s->xy_size;
    s->stroke_sizes[s->num_strokes-1] = 0;
}

int get_x(struct Strokes *s, int stroke, int point) {
    return s->x[s->stroke_offsets[stroke] + point];
}

int get_y(struct Strokes *s, int stroke, int point) {
    return s->y[s->stroke_offsets[stroke] + point];
}

void print_strokes(struct Strokes *s) {
    for (int i = 0; i < s->num_strokes; ++i) {
        printf("Stroke %d starts on index %d with a length of %d and a capacity of %d\n", i + 1, s->stroke_offsets[i], s->stroke_sizes[i], s->xy_capacity);
        for (int j = 0; j < s->stroke_sizes[i]; ++j) {
            printf("(%d, %d) ", get_x(s, i, j), get_y(s, i, j));
        }
        printf("\n\n");
    }
}

int main(void) {
    InitWindow(WIDTH, HEIGHT, "Draw");
    SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));
    
    bool highlight_mode = false;

    RenderTexture2D render_texture = LoadRenderTexture(WIDTH, HEIGHT);

    struct Strokes strokes = {0};

    BeginTextureMode(render_texture);
    ClearBackground(WHITE);
    EndTextureMode();
    
    int pX = 0;
    int pY = 0;

    int hX = 0;
    int hY = 0;

    while(!WindowShouldClose()) {
        int x = GetMouseX();
        if (x > WIDTH) x = WIDTH;
        else if (x < 0) x = 0;
        
        int y = GetMouseY();
        if (y > HEIGHT) y = HEIGHT;
        else if (y < 0) y = 0;

        if (IsKeyPressed(KEY_H)) {
            if (!highlight_mode) {
                highlight_mode = true;
                hX = GetMouseX();
                hY = GetMouseY();                
            } else {
                highlight_mode = false;                
            }
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            add_stroke(&strokes);
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            BeginTextureMode(render_texture);
            DrawLineEx((Vector2){pX, pY}, (Vector2){x, y}, 2, BLACK);
            EndTextureMode();

            add_point(&strokes, x, y);
        }

        BeginDrawing();
        {
            ClearBackground(RAYWHITE);

            // NOTE: Render texture must be y-flipped due to default OpenGL coordinates (left-bottom)
            DrawTextureRec(render_texture.texture, (Rectangle) { 0, 0, (float)render_texture.texture.width, (float)-render_texture.texture.height }, (Vector2) { 0, 0 }, WHITE);

            if (highlight_mode) {
                int w = x - hX;
                int h = y - hY;
                int ax = w < 0 ? x : hX;
                int ay = h < 0 ? y : hY;

                w = abs(w);
                h = abs(h);

                DrawRectangleLines(ax, ay, w, h, BLACK);
                /* DrawText(TextFormat("HIGHLIGHT MODE: %d", in_the_highlight(ax, ay, w, h, head)), 10, 50, 20, BLACK); */
            }

            DrawText(TextFormat("Stroke: %d", strokes.num_strokes), 10, 10, 20, BLACK);
        }
        EndDrawing();

        pX = x;
        pY = y;
    }

    UnloadRenderTexture(render_texture);

    CloseWindow();

    print_strokes(&strokes);
}
