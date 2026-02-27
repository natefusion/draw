#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

#define WIDTH 1000
#define HEIGHT 1000

struct Point {
    int x;
    int y;
};

struct Stroke_Slice {
    int offset;
    int size;
};

struct Box {
    int x;
    int y;
    RenderTexture2D texture;
};

struct Strokes {
    int xy_size;
    int xy_capacity;
    int num_strokes;
    int strokes_capacity;
    struct Stroke_Slice *slices;
    struct Box *boxes;
    struct Point *xy;
};

void add_point(struct Strokes *s, int x, int y) {
    if (s->xy_size >= 1 && s->xy[s->xy_size - 1].x == x && s->xy[s->xy_size - 1].y == y) return;
    
    s->xy_size += 1;
    s->slices[s->num_strokes-1].size += 1;

    if (s->xy_size >= s->xy_capacity) {
        s->xy_capacity = 1 + (int)roundf(1.618033989f * (float)s->xy_capacity);
        s->xy = realloc(s->xy, s->xy_capacity * sizeof(struct Point));
    }

    s->xy[s->xy_size - 1].x = x;
    s->xy[s->xy_size - 1].y = y;
}

void add_stroke(struct Strokes *s) {
    s->num_strokes += 1;
    if (s->num_strokes >= s->strokes_capacity) {
        s->strokes_capacity = 1 + (int)roundf(1.618033989f * (float)s->strokes_capacity);
        s->slices         = realloc(s->slices, s->strokes_capacity * sizeof(struct Stroke_Slice));
        s->boxes          = realloc(s->boxes, s->strokes_capacity * sizeof(struct Box));
    }

    s->slices[s->num_strokes-1].offset = s->xy_size;
    s->slices[s->num_strokes-1].size = 0;
    s->boxes[s->num_strokes-1] = (struct Box){0};
}

void draw_stroke(struct Strokes *s, int stroke, int x, int y, Color color) {
    if (stroke < 0) return;
    int height = s->boxes[stroke].texture.texture.height;
    
    int idx = s->slices[stroke].offset + 0;
    Vector2 prev = {s->xy[idx].x + x, height - s->xy[idx].y + y};

    for (int i = 1; i < s->slices[stroke].size; ++i) {
        int idx = s->slices[stroke].offset + i;
        Vector2 cur = {s->xy[idx].x + x, height - s->xy[idx].y + y};
        DrawLineEx(prev, cur, 2, color);
        prev = cur;
    }
}

void upload_stroke_to_gpu_memory(struct Strokes *s) {
    struct Box *box = &s->boxes[s->num_strokes-1];

    int x_min = INT_MAX;
    int y_min = INT_MAX;
    int x_max = INT_MIN;
    int y_max = INT_MIN;
    for (int i = 0; i < s->slices[s->num_strokes-1].size; ++i) {
        int idx = s->slices[s->num_strokes-1].offset + i; 
        if (s->xy[idx].x < x_min) x_min = s->xy[idx].x;
        if (s->xy[idx].x > x_max) x_max = s->xy[idx].x;
        if (s->xy[idx].y < y_min) y_min = s->xy[idx].y;
        if (s->xy[idx].y > y_max) y_max = s->xy[idx].y;
    }

    x_min -= 1;
    x_max += 1;
    y_min -= 1;
    y_max += 1;


    box->x = x_min;
    box->y = y_min;

    int width = x_max - x_min;
    int height = y_max - y_min;

    printf("Width is %d and height is %d\n", width, height);
    printf("x is %d and y is %d\n", box->x, box->y);

    RenderTexture2D t = LoadRenderTexture(width, height);

    BeginTextureMode(t);


    int idx = s->slices[s->num_strokes-1].offset + 0;
    s->xy[idx].x -= x_min;
    s->xy[idx].y = y_max - s->xy[idx].y;
    Vector2 prev = {s->xy[idx].x, s->xy[idx].y};
    
    for (int i = 1; i < s->slices[s->num_strokes-1].size; ++i) {
        int idx = s->slices[s->num_strokes-1].offset + i;
        s->xy[idx].x -= x_min;
        s->xy[idx].y = y_max - s->xy[idx].y;
        Vector2 cur = {s->xy[idx].x, s->xy[idx].y};
        DrawLineEx(prev, cur, 2, BLACK);
        prev = cur;
    }

    EndTextureMode();
    
    box->texture = t;
}

int stroke_collision(struct Strokes *s, int x, int y) {
    Vector2 pos = (Vector2){.x=(float)x, .y=(float)y};
    for (int i = 0; i < s->num_strokes; ++i) {
        Rectangle r = {
            .x=s->boxes[i].x,
            .y=s->boxes[i].y,
            .width=s->boxes[i].texture.texture.width,
            .height=s->boxes[i].texture.texture.height,
        };
        
        if (CheckCollisionPointRec(pos, r)) {
            return i;
        }
    }

    return -1;
}

void print_strokes(struct Strokes *s) {
    for (int i = 0; i < s->num_strokes; ++i) {
        printf("Stroke %d starts on index %d with a length of %d and a capacity of %d\n", i + 1, s->slices[i].offset, s->slices[i].size, s->xy_capacity);
        for (int j = 0; j < s->slices[i].size; ++j) {
            int idx = s->slices[i].offset + j;
            printf("(%d, %d) ", s->xy[idx].x, s->xy[idx].y);
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

    int stroke_idx = -1;

    int offsetX = 0;
    int offsetY = 0;

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
            add_point(&strokes, pX, pY);
        } else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            upload_stroke_to_gpu_memory(&strokes);
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
            }

            if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
                if (stroke_idx == -1) {
                    stroke_idx = stroke_collision(&strokes, x, y);
                    BeginTextureMode(render_texture);
                    draw_stroke(&strokes, stroke_idx, strokes.boxes[stroke_idx].x, strokes.boxes[stroke_idx].y, WHITE);
                    EndTextureMode();
                    offsetX = x - strokes.boxes[stroke_idx].x; // so you pick it up where the mouse cursor is, rather than it snapping to the corner
                    offsetY = y - strokes.boxes[stroke_idx].y;
                    printf("Stroke %d has been selected\n", stroke_idx);
                } else {
                    BeginTextureMode(render_texture);
                    draw_stroke(&strokes, stroke_idx, x - offsetX, y - offsetY, BLACK);
                    EndTextureMode();
                    strokes.boxes[stroke_idx].x = x - offsetX;
                    strokes.boxes[stroke_idx].y = y - offsetY;
                    printf("Stroke %d has been unselected\n", stroke_idx);
                    stroke_idx = -1;
                }
            }
            
            if (stroke_idx != -1) {
                DrawTextureEx(strokes.boxes[stroke_idx].texture.texture, (Vector2){x - offsetX, y - offsetY}, 0.0f, 1.0f, WHITE);
            }
            
            DrawText(TextFormat("Stroke: %d", strokes.num_strokes), 10, 10, 20, BLACK);
        }
        EndDrawing();

        pX = x;
        pY = y;
    }

    for (int i = 0; i < strokes.num_strokes; ++i) {
        UnloadRenderTexture(strokes.boxes[i].texture);
    }

    UnloadRenderTexture(render_texture);

    CloseWindow();

    print_strokes(&strokes);
}
