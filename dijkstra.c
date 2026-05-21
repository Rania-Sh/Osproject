#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>

#include "animation.h"
#define MAX_NODES 100

int dijkstra(Graph *g, int src, int dst, int *path);
void printPath(int parent[], int j, bool first) {
    if (parent[j] == -1) {
        printf("%d", j);
        return;
    }
    printPath(parent, parent[j], false);
    printf("->%d", j);
}

int dijkstra(Graph *g, int src, int dst, int *path) {
    int n = g->numVertices;
    int dist[64], prev[64], visited[64];
    for (int i = 0; i < n; i++) {
        dist[i] = 1000000; prev[i] = -1; visited[i] = 0;
    }
    dist[src] = 0;
    for (int iter = 0; iter < n; iter++) {
        int u = -1;
        for (int i = 0; i < n; i++)
            if (!visited[i] && (u == -1 || dist[i] < dist[u])) u = i;
        if (u == -1 || dist[u] == 1000000) break;
        visited[u] = 1;
        for (int v = 0; v < n; v++) {
            if (g->weights[u][v] > 0) {
                int nd = dist[u] + g->weights[u][v];
                if (nd < dist[v]) { dist[v] = nd; prev[v] = u; }
            }
        }
    }
    int tmp[64], len = 0, cur = dst;
    while (cur != -1) { tmp[len++] = cur; cur = prev[cur]; }
    for (int i = 0; i < len; i++) path[i] = tmp[len - 1 - i];
    return (len > 0 && path[0] == src) ? len : 0;
}