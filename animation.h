#ifndef OSPROJECT_ANIMATION_H
#define OSPROJECT_ANIMATION_H

#include <stdbool.h>

typedef struct {
    int to;
    int weight;
} Edge;

typedef struct {
    int numVertices;
    int **weights;
} Graph;

typedef struct {
    float x;
    float y;
} Point;

typedef struct {
    Point position;
    int currentNode;
    bool isNodePause;
    float duration;
} AnimationStep;

typedef struct {
    AnimationStep *steps;
    int count;
    int capacity;
    int currentStep;
    float elapsed;
    bool finished;
} AnimationPath;

AnimationPath *createAnimationPath(void);
void addAnimationStep(AnimationPath *path, Point position, int currentNode,
                      bool isNodePause, float duration);
AnimationPath *buildAnimationPath(Graph *graph, int *shortestPath,
                                  int pathLength, Point *nodePositions);
void updateAnimation(AnimationPath *path, float deltaTime,
                     Point *currentPosition);
void freeAnimationPath(AnimationPath *path);

#endif //OSPROJECT_ANIMATION_H
