#ifndef OSPROJECT_ANIMATIONUI_H
#define OSPROJECT_ANIMATIONUI_H

#include "raylib.h"
#include <stdbool.h>
#include "animation.h"

/* קריאת גרף בלבד (תאימות לאחור) */
Graph *loadGraph(const char *filename, int *numNodes);

/* קריאת גרף + נוסעים מקובץ מורחב */
Graph *loadGraphAndTravelers(const char *filename, int *numNodes,
                              int *numTravelers,
                              int *sources, int *dests);

void freeGraph(Graph *g);
void defaultLayout(int n, Point *pos);
bool drawButton(Rectangle rect, const char *label,
                Color bg, Color fg, bool hover);

#endif /* OSPROJECT_ANIMATIONUI_H */
