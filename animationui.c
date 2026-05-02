 //
// Created by student on 01/05/2026.
//
#include "animationui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>




Graph *loadGraph(const char *filename, int *numNodes) {
    FILE *f = fopen(filename, "r");
    if (!f) return NULL;

    int n;
    fscanf(f, "%d", &n);
    *numNodes = n;

    Graph *g = malloc(sizeof(Graph));
    g->numVertices = n;

    g->weights = malloc(n * sizeof(int *));
    for (int i = 0; i < n; i++)
        g->weights[i] = calloc(n, sizeof(int));

    int a, b, w;
    while (fscanf(f, "%d %d %d", &a, &b, &w) == 3) {
        g->weights[a][b] = w;
        g->weights[b][a] = w;
    }

    fclose(f);
    return g;
}

void freeGraph(Graph *g) {
    if (!g) return;
    for (int i = 0; i < g->numVertices; i++)
        free(g->weights[i]);
    free(g->weights);
    free(g);
}







void defaultLayout(int n, Point *pos) {
    float cx = 450, cy = 325, r = 220;

    for (int i = 0; i < n; i++) {
        float angle = (2 * 3.14159f * i) / n;
        pos[i].x = cx + r * cosf(angle);
        pos[i].y = cy + r * sinf(angle);
    }
}






bool drawButton(Rectangle rect, const char *label,
                Color bg, Color fg, bool hover) {

    Color c = hover ? ColorBrightness(bg, 0.2f) : bg;

    DrawRectangleRounded(rect, 0.3f, 8, c);

    int tw = MeasureText(label, 20);
    DrawText(label,
             rect.x + (rect.width - tw) / 2,
             rect.y + 10,
             20, fg);

    return hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

