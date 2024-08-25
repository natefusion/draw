#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>

enum { RADIUS = 4 };

struct Point {
    int x;
    int y;
    struct Point *next;
};

struct Point *make_point(int x, int y) {
    struct Point *p = (struct Point *)malloc(sizeof(struct Point));
    p->x = x;
    p->y = y;
    return p;
}

struct Stroke {
    int min_x;
    int min_y;
    int max_x;
    int max_y;
    struct Point *points;
    struct Stroke *next;
};

struct Stroke *make_stroke() {
    struct Stroke *s = (struct Stroke *)malloc(sizeof(struct Stroke));
    s->min_x = 0;
    s->min_y = 0;
    s->max_x = 0;
    s->max_y = 0;
    s->points = NULL;
    s->next = NULL;
    return s;
}

void push_point(struct Stroke *s, int x, int y) {
    struct Point *p = make_point(x, y);
    p->next = s->points;
    s->points = p;
}

void free_points(struct Point *p) {
    if (!p)
        return;

    struct Point *n = p->next;
    free(p);
    free_points(n);
}

void free_strokes(struct Stroke *s) {
    if (!s)
        return;

    struct Stroke *n = s->next;
    free_points(s->points);
    free(s);
    free_strokes(n);
}

void print_points(struct Point *p) {
    while (p) {
        printf("(%d, %d) ", p->x, p->y);
        p = p->next;
    }
    printf("\n");
}

void print_strokes(struct Stroke *s) {
    while(s) {
        printf("(min x, min y) = (%d, %d)\n", s->min_x, s->min_y);
        printf("(max x, max y) = (%d, %d)\n", s->max_x, s->max_y);
        print_points(s->points);
        s = s->next;
        printf("\n");
    }

}

void get_min_max_for_stroke(struct Stroke *s) {
    if (!s)
        return;
    struct Point *p = s->points;
    if (!p)
        return;
    
    int min_x = p->x;
    int min_y = p->y;
    int max_x = min_x;
    int max_y = min_y;

    while (p) {
        if (p->x < min_x)
            min_x = p->x;
        else if (p->x > max_x)
            max_x = p->x;

        if (p->y < min_y)
            min_y = p->y;
        else if (p->y > max_y)
            max_y = p->y;
        
        p = p->next;
    }

    s->min_x = min_x;
    s->min_y = min_y;
    s->max_x = max_x;
    s->max_y = max_y;
}

int in_the_highlight(int hX, int hY, int w, int h, struct Stroke *s) {
    int stroke_num = 0;

    while(s) {
        if (s->min_x >= hX && s->max_x <= (hX+w) && s->min_y >= hY && s->max_y <= (hY+h))
            return stroke_num;

        s = s->next;
        stroke_num++;
    }

    return -1;
}

void dot(int x, int y) {
    DrawCircle(x, y, RADIUS, BLACK);
}

void erase(int x, int y) {
    DrawCircle(x, y, RADIUS * 2 , RAYWHITE);
}

void swap() {
    SwapScreenBuffer();
}

void clear() {
    ClearBackground(RAYWHITE);
}


void lerp(int x1, int y1, int x2, int y2) {
    DrawLineEx((Vector2){x1, y1}, (Vector2){x2, y2}, RADIUS, BLACK);
    /* DrawLine(x1, y1, x2, y2, BLACK); */
}

int main() {
    InitWindow(1000, 1000, "Draw");
    double previousTime = GetTime();
    double currentTime = 0.0;
    double updateDrawTime = 0.0;
    int targetFPS = 60;
    bool highlight_mode = false;
    int stroke_num = 0;

    struct Stroke *head = NULL;

    int pX = 0;
    int pY = 0;

    int hX = 0;
    int hY = 0;

    clear();

    while(!WindowShouldClose()) {
        PollInputEvents();

        if (IsKeyPressed(KEY_E)) {
            clear();
        }

        if (IsKeyPressed(KEY_S)) {
            swap();
        }

        if (IsKeyPressed(KEY_H)) {
            highlight_mode = !highlight_mode;
            hX = GetMouseX();
            hY = GetMouseY();
        }

        BeginDrawing();
        {
            int x = GetMouseX();
            int y = GetMouseY();

            if (highlight_mode) {
                int w = x - hX;
                int h = y - hY;
                int ax = w < 0 ? x : hX;
                int ay = h < 0 ? y : hY;

                w = abs(w);
                h = abs(h);

                DrawRectangle(10, 50, 220, 20, RAYWHITE);
                DrawText(TextFormat("HIGHLIGHT MODE: %d", in_the_highlight(ax, ay, w, h, head)), 10, 50, 20, BLACK);
                DrawRectangle(ax, ay, w, h, RAYWHITE);
                DrawRectangleLines(ax, ay, w, h, BLACK);
            } else {
                if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(0) || IsMouseButtonPressed(4)) {
                    get_min_max_for_stroke(head);
                    stroke_num++;
                    struct Stroke *new_stroke = make_stroke();
                    new_stroke->next = head;
                    head = new_stroke;
                }
                
                if (IsKeyDown(KEY_SPACE) || IsMouseButtonDown(0) || IsMouseButtonDown(4)) {
                    lerp(pX, pY, x, y);
                    push_point(head, x, y);
                } else if (IsKeyDown(KEY_C) || IsMouseButtonDown(1)) {
                    erase(x, y);
                }
            
                DrawRectangle(10, 10, 110, 20, RAYWHITE);
                DrawText(TextFormat("Stroke: %d", stroke_num), 10, 10, 20, BLACK);
            }

            pX = x;
            pY = y;
        }
        EndDrawing();
        swap();

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
    print_strokes(head);
    free_strokes(head);
}
