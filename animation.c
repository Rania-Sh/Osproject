//
// Created by student on 01/05/2026.
//
#include <stdlib.h>
#include <stdio.h>
#include "animation.h"

#define INITIAL_CAPACITY 32
#define EDGE_STEP_TIME 0.3f
#define NODE_PAUSE_TIME 1.0f

AnimationPath *createAnimationPath(void) {
    AnimationPath *path = malloc(sizeof(AnimationPath));
    if (!path) return NULL;

    path->steps = malloc(INITIAL_CAPACITY * sizeof(AnimationStep));
    if (!path->steps) {
        free(path);
        return NULL;
    }
    path->count = 0;
    path->capacity = INITIAL_CAPACITY;
    path->currentStep = 0;
    path->elapsed = 0.0f;
    path->finished = false;
    return path;
}

void addAnimationStep(AnimationPath *path, Point position, int currentNode,
                      bool isNodePause, float duration) {
    if (!path) return;

    if (path->count >= path->capacity) {
        path->capacity *= 2;
        AnimationStep *newSteps = (AnimationStep *)malloc(path->capacity * sizeof(AnimationStep));
        if (!newSteps) {
            fprintf(stderr, "Memory allocation failed");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < path->count; i++) {
            newSteps[i] = path->steps[i];
        }

        free(path->steps);
        path->steps = newSteps;
    }

    path->steps[path->count].position = position;
    path->steps[path->count].currentNode = currentNode;
    path->steps[path->count].isNodePause = isNodePause;
    path->steps[path->count].duration = duration;
    path->count++;
}

static Point interpolate(Point a, Point b, float t) {
    Point p;
    p.x = a.x + (b.x - a.x) * t;
    p.y = a.y + (b.y - a.y) * t;
    return p;
}

AnimationPath *buildAnimationPath(Graph *graph, int *shortestPath,
                                  int pathLength, Point *nodePositions) {
    if (!graph || !shortestPath || pathLength <= 0 || !nodePositions)
        return NULL;

    AnimationPath *anim = createAnimationPath();
    if (!anim) return NULL;

    addAnimationStep(anim,
                     nodePositions[shortestPath[0]],
                     shortestPath[0],
                     false,
                     0.0f);
    for (int i = 0; i < pathLength - 1; i++) {
          int from = shortestPath[i];
          int to = shortestPath[i + 1];
          int weight = graph->weights[from][to];

          Point start = nodePositions[from];
          Point end = nodePositions[to];

        for (int step = 1; step <= weight; step++) {
             float t = (float)step / (float)weight;
             Point pos = interpolate(start, end, t);
             addAnimationStep(anim, pos, -1, false, EDGE_STEP_TIME);
        }

        if (i < pathLength - 2) {
              addAnimationStep(anim,
                               nodePositions[to],
                               to,
                               true,
                                 NODE_PAUSE_TIME);
        }
    }

addAnimationStep(anim,
                 nodePositions[shortestPath[pathLength - 1]],
                 shortestPath[pathLength - 1],
                 false,
                 0.0f);

return anim;
}
void updateAnimation(AnimationPath *path, float deltaTime,
                     Point *currentPosition) {
    if (!path || !currentPosition || path->finished || path->count == 0)
        return;

    if (path->currentStep >= path->count) {
        path->finished = true;
        return;
    }

    AnimationStep *step = &path->steps[path->currentStep];
    *currentPosition = step->position;

    if (step->duration <= 0.0f) {
        path->currentStep++;
        if (path->currentStep >= path->count)
            path->finished = true;
        return;
    }
    path->elapsed += deltaTime;

    if (path->elapsed >= step->duration) {
        path->elapsed = 0.0f;
        path->currentStep++;
        if (path->currentStep >= path->count)
            path->finished = true;
    }
}

void freeAnimationPath(AnimationPath *path) {
    if (!path) return;
    free(path->steps);
    free(path);
}