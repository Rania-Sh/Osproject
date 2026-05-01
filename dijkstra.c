#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>

#define MAX_NODES 100

void printPath(int parent[], int j, bool first) {
    if (parent[j] == -1) {
        printf("%d", j);
        return;
    }
    printPath(parent, parent[j], false);
    printf("->%d", j);
}

void dijkstra(int graph[MAX_NODES][MAX_NODES], int n, int src, int dest) {
    int dist[MAX_NODES];
    bool sptSet[MAX_NODES];
    int parent[MAX_NODES];

    for (int i = 0; i < n; i++) {
        dist[i] = INT_MAX;
        sptSet[i] = false;
        parent[i] = -1;
    }

    dist[src] = 0;

    for (int count = 0; count < n - 1; count++) {
        int u = -1;
        for (int v = 0; v < n; v++)
            if (!sptSet[v] && (u == -1 || dist[v] < dist[u]))
                u = v;

        if (dist[u] == INT_MAX) break;
        sptSet[u] = true;

        for (int v = 0; v < n; v++) {
            if (!sptSet[v] && graph[u][v] != -1 && dist[u] + graph[u][v] < dist[v]) {
                parent[v] = u;
                dist[v] = dist[u] + graph[u][v];
            }
        }
    }

    if (dist[dest] == INT_MAX) {
        printf("No path found\n");
    } else {
        printPath(parent, dest, true);
        printf("\n%d\n", dist[dest]);
    }
}

