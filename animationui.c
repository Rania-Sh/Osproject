 //
// Created by student on 01/05/2026.
//
#include "animationui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ── loadGraph המקורי (נשאר לתאימות) ── */
Graph *loadGraph(const char *filename, int *numNodes) {
    FILE *f = fopen(filename, "r");
    if (!f) return NULL;

    /* דלג על שורות הערה */
    char line[256];
    int n = -1, edges = -1;

    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '#' || line[0] == '\n') continue;
        if (n == -1) {
            sscanf(line, "%d %d", &n, &edges);
            break;
        }
    }
    if (n == -1) { fclose(f); return NULL; }

    *numNodes = n;
    Graph *g = malloc(sizeof(Graph));
    g->numVertices = n;
    g->weights = malloc(n * sizeof(int *));
    for (int i = 0; i < n; i++)
        g->weights[i] = calloc(n, sizeof(int));

    int a, b, w;
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '#' || line[0] == '\n') break;
        if (sscanf(line, "%d %d %d", &a, &b, &w) == 3) {
            g->weights[a][b] = w;
            g->weights[b][a] = w;
        }
    }
    fclose(f);
    return g;
}

/*
 * ── loadGraphAndTravelers ──
 * קורא גרף ורשימת נוסעים מקובץ בפורמט:
 *
 *   # graph definition
 *   <N> <E>
 *   <a> <b> <w>
 *   ...
 *   # travelers
 *   <numTravelers>
 *   <src> <dst>
 *   ...
 */
Graph *loadGraphAndTravelers(const char *filename, int *numNodes,
                              int *numTravelers,
                              int *sources, int *dests) {
    FILE *f = fopen(filename, "r");
    if (!f) return NULL;

    char line[256];
    int  n = -1;
    int  section = 0; /* 0=graph, 1=travelers */
    int  edgesRead = 0, edgesTotal = -1;
    int  travelersRead = 0;
    *numTravelers = 0;

    Graph *g = NULL;

    while (fgets(line, sizeof(line), f)) {
        /* הערות */
        if (line[0] == '#') {
            if (strstr(line, "travelers")) section = 1;
            continue;
        }
        /* שורה ריקה */
        if (line[0] == '\n' || line[0] == '\r') continue;

        if (section == 0) {
            if (n == -1) {
                /* שורה ראשונה: N E */
                sscanf(line, "%d %d", &n, &edgesTotal);
                *numNodes = n;
                g = malloc(sizeof(Graph));
                g->numVertices = n;
                g->weights = malloc(n * sizeof(int *));
                for (int i = 0; i < n; i++)
                    g->weights[i] = calloc(n, sizeof(int));
            } else {
                int a, b, w;
                if (sscanf(line, "%d %d %d", &a, &b, &w) == 3) {
                    g->weights[a][b] = w;
                    g->weights[b][a] = w;
                    edgesRead++;
                }
            }
        } else {
            /* section == 1: travelers */
            if (*numTravelers == 0) {
                sscanf(line, "%d", numTravelers);
            } else {
                int s, d;
                if (sscanf(line, "%d %d", &s, &d) == 2) {
                    sources[travelersRead] = s;
                    dests[travelersRead]   = d;
                    travelersRead++;
                }
            }
        }
    }
    fclose(f);

    if (!g || *numTravelers == 0) {
        if (g) freeGraph(g);
        return NULL;
    }
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
             rect.x + (rect.width  - tw) / 2,
             rect.y + 10,
             20, fg);
    return hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}
