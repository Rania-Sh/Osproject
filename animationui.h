//
// Created by student on 01/05/2026.
//

#ifndef OSPROJECT_ANIMATIONUI_H
#define OSPROJECT_ANIMATIONUI_H


#include "raylib.h"
#include <stdbool.h>


#include "animation.h"

Graph *loadGraph(const char *filename, int *numNodes);
void freeGraph(Graph *g);




void defaultLayout(int n, Point *pos);

bool drawButton(Rectangle rect, const char *label,
                Color bg, Color fg, bool hover);


#endif //OSPROJECT_ANIMATIONUI_H
