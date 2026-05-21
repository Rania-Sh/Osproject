#include "raylib.h"
#include "animationui.h"
#include "animation.h"
#include <stdio.h>
#include <math.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/wait.h>

int dijkstra(Graph *g, int src, int dst, int *path);
#define SCREEN_W 900
#define SCREEN_H 650
#define MAX_TRAVELERS 16

typedef enum {
    STATE_IDLE,
    STATE_RUNNING,
    STATE_PAUSED,
    STATE_FINISHED
} AnimState;

/* ── צבעים שונים לכל נוסע ── */
static Color TRAVELER_COLORS[MAX_TRAVELERS] = {
    {220,  50,  50, 255},  /* אדום       */
    { 50, 200,  50, 255},  /* ירוק       */
    { 50, 130, 255, 255},  /* כחול       */
    {255, 200,   0, 255},  /* צהוב       */
    {200,  50, 220, 255},  /* סגול       */
    {  0, 220, 200, 255},  /* טורקיז     */
    {255, 130,   0, 255},  /* כתום       */
    {255, 100, 180, 255},  /* ורוד        */
    {100, 255, 130, 255},  /* ירוק בהיר  */
    {130, 180, 255, 255},  /* כחול בהיר  */
    {255,  80,  80, 255},
    { 80, 255, 255, 255},
    {255, 255,  80, 255},
    {180,  80, 255, 255},
    {255, 180,  80, 255},
    { 80, 255, 180, 255},
};

void DrawArrowLine(Vector2 start, Vector2 end, float thickness, Color color);

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    /* ── קריאת גרף ונוסעים מהקובץ ── */
    int n;
    int numTravelers;
    int sources[MAX_TRAVELERS], dests[MAX_TRAVELERS];

    Graph *g = loadGraphAndTravelers(argv[1], &n, &numTravelers, sources, dests);
    if (!g) {
        fprintf(stderr, "Failed to load graph from %s\n", argv[1]);
        return 1;
    }

    /* ── חישוב מסלולי דייקסטרה לכל נוסע (האב עושה הכל) ── */
    int   paths[MAX_TRAVELERS][64];
    int   pathLens[MAX_TRAVELERS];
    Point pos[64];
    defaultLayout(n, pos);

    AnimationPath *anims[MAX_TRAVELERS];
    Point          agents[MAX_TRAVELERS];

    for (int t = 0; t < numTravelers; t++) {
        pathLens[t] = dijkstra(g, sources[t], dests[t], paths[t]);
        if (pathLens[t] == 0) {
            fprintf(stderr, "No path for traveler %d (%d->%d)\n",
                    t, sources[t], dests[t]);
            freeGraph(g);
            return 1;
        }
        anims[t]  = buildAnimationPath(g, paths[t], pathLens[t], pos);
        agents[t] = pos[paths[t][0]];
    }

    /* ── יצירת תהליכי הבנים (fork) ── */
    pid_t childPids[MAX_TRAVELERS];

    for (int t = 0; t < numTravelers; t++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork failed");
            freeGraph(g);
            return 1;
        }
        if (pid == 0) {
            /* ── תהליך בן ── */
            printf("[%d] started\n", getpid());
            fflush(stdout);
            /* הבן ישן עד שהאב ישלח לו SIGTERM בסיום מסלולו */
            while (1) pause();
            exit(0);
        }
        /* האב שומר את ה-PID */
        childPids[t] = pid;
    }

    /* ── לולאת raylib (רק האב) ── */
    InitWindow(SCREEN_W, SCREEN_H, "Graph Animation System - Multi Traveler");
    SetTargetFPS(60);

    AnimState state = STATE_IDLE;
    bool finished[MAX_TRAVELERS];
    bool signalSent[MAX_TRAVELERS];
    for (int t = 0; t < numTravelers; t++) {
        finished[t]   = false;
        signalSent[t] = false;
    }

    Rectangle btnAction  = {  25, 25, 120, 40 };
    Rectangle btnRestart = {  25, 75, 120, 40 };

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        Vector2 mousePos    = GetMousePosition();
        bool hoverAction    = CheckCollisionPointRec(mousePos, btnAction);
        bool hoverRestart   = CheckCollisionPointRec(mousePos, btnRestart);

        /* כפתורי Play / Stop / Restart */
        if (state == STATE_RUNNING) {
            if (drawButton(btnAction, "Stop", ORANGE, WHITE, hoverAction))
                state = STATE_PAUSED;
        } else {
            if (drawButton(btnAction, "Play", LIME, DARKGRAY, hoverAction)) {
                if (state == STATE_FINISHED) {
                    /* איפוס כל הנוסעים */
                    for (int t = 0; t < numTravelers; t++) {
                        anims[t]->currentStep = 0;
                        anims[t]->elapsed     = 0.0f;
                        anims[t]->finished    = false;
                        agents[t]             = pos[paths[t][0]];
                        finished[t]           = false;
                    }
                }
                state = STATE_RUNNING;
            }
        }

        if (drawButton(btnRestart, "Restart", MAROON, WHITE, hoverRestart)) {
            state = STATE_IDLE;
            for (int t = 0; t < numTravelers; t++) {
                anims[t]->currentStep = 0;
                anims[t]->elapsed     = 0.0f;
                anims[t]->finished    = false;
                agents[t]             = pos[paths[t][0]];
                finished[t]           = false;
            }
        }

        /* עדכון אנימציות + שליחת סיגנל לבן שסיים */
        if (state == STATE_RUNNING) {
            int allDone = 1;
            for (int t = 0; t < numTravelers; t++) {
                if (!finished[t]) {
                    updateAnimation(anims[t], dt, &agents[t]);
                    if (anims[t]->finished) {
                        finished[t] = true;
                        /* שליחת SIGTERM לבן המתאים */
                        if (!signalSent[t]) {
                            kill(childPids[t], SIGTERM);
                            signalSent[t] = true;
                        }
                    }
                    allDone = 0;
                }
            }
            if (allDone) state = STATE_FINISHED;
        }

        /* ── ציור ── */
        BeginDrawing();
        ClearBackground((Color){ 20, 30, 45, 255 });

        /* קווי גרף */
        for (int i = 0; i < n; i++) {
            for (int j = i + 1; j < n; j++) {
                if (g->weights[i][j] > 0) {
                    Vector2 start = { pos[i].x, pos[i].y };
                    Vector2 end   = { pos[j].x, pos[j].y };
                    DrawArrowLine(start, end, 1.5f, Fade(LIGHTGRAY, 0.5f));

                    float dx = end.x - start.x, dy = end.y - start.y;
                    float len = sqrtf(dx*dx + dy*dy);
                    float midX = (start.x + end.x) / 2.0f;
                    float midY = (start.y + end.y) / 2.0f;
                    float ox = (len != 0.0f) ? -dy/len * 22.0f : 0.0f;
                    float oy = (len != 0.0f) ?  dx/len * 22.0f : 0.0f;

                    const char *wt = TextFormat("%d", g->weights[i][j]);
                    int fs = 18, tw = MeasureText(wt, fs);
                    DrawRectangle(midX+ox-tw/2-5, midY+oy-fs/2-3, tw+10, fs+6, Fade(BLACK,0.75f));
                    DrawText(wt, midX+ox-tw/2, midY+oy-fs/2, fs, YELLOW);
                }
            }
        }

        /* מסלולי כל הנוסעים (צבע שונה לכל אחד) */
        for (int t = 0; t < numTravelers; t++) {
            Color col = TRAVELER_COLORS[t % MAX_TRAVELERS];
            Color dark = Fade(col, 0.4f);
            for (int k = 0; k < pathLens[t] - 1; k++) {
                int from = paths[t][k], to = paths[t][k+1];
                Vector2 start = { pos[from].x, pos[from].y };
                Vector2 end   = { pos[to].x,   pos[to].y   };
                DrawArrowLine(start, end, 7.0f, dark);
                DrawArrowLine(start, end, 4.0f, col);
            }
        }

        /* צמתים */
        for (int i = 0; i < n; i++) {
            DrawCircle(pos[i].x, pos[i].y, 20, RAYWHITE);
            DrawCircleLines(pos[i].x, pos[i].y, 15, LIGHTGRAY);
            DrawText(TextFormat("%d", i), pos[i].x - 6, pos[i].y - 8, 18, BLACK);
        }

        /* סוכנים — כל נוסע בצבע שלו */
        for (int t = 0; t < numTravelers; t++) {
            Color col = TRAVELER_COLORS[t % MAX_TRAVELERS];
            Vector2 ap = { agents[t].x, agents[t].y };
            DrawCircleV(ap, 20, Fade(col, 0.25f));
            DrawCircleV(ap, 13, col);
            DrawCircleLinesV(ap, 15, WHITE);
            DrawText(TextFormat("%d", t), ap.x - 4, ap.y - 8, 14, BLACK);
        }

        /* מקרא צבעים בצד ימין */
        for (int t = 0; t < numTravelers; t++) {
            Color col = TRAVELER_COLORS[t % MAX_TRAVELERS];
            int legendY = 25 + t * 28;
            DrawCircle(SCREEN_W - 130, legendY + 10, 10, col);
            DrawText(TextFormat("T%d: %d->%d [PID:%d]",
                                t, sources[t], dests[t], (int)childPids[t]),
                     SCREEN_W - 115, legendY, 14, col);
        }

        /* סטטוס */
        if (state == STATE_RUNNING)
            DrawText("STATUS: RUNNING",  160, 35, 18, LIME);
        else if (state == STATE_PAUSED)
            DrawText("STATUS: PAUSED",   160, 35, 18, ORANGE);
        else if (state == STATE_FINISHED) {
            DrawText("STATUS: FINISHED", 160, 35, 18, SKYBLUE);
            DrawText("All Travelers Reached Destination!", 220, 585, 24, RAYWHITE);
        } else
            DrawText("STATUS: READY",    160, 35, 18, LIGHTGRAY);

        EndDrawing();
    }

    /* ── האב ממתין לכל הבנים ── */
    for (int t = 0; t < numTravelers; t++) {
        /* שליחת סיגנל לבנים שעדיין לא קיבלו (אם המשתמש סגר חלון) */
        if (!signalSent[t]) kill(childPids[t], SIGTERM);
        waitpid(childPids[t], NULL, 0);
    }

    /* ── ניקוי ── */
    for (int t = 0; t < numTravelers; t++)
        freeAnimationPath(anims[t]);
    freeGraph(g);
    CloseWindow();
    return 0;
}

/* ── DrawArrowLine (ללא שינוי) ── */
void DrawArrowLine(Vector2 start, Vector2 end, float thickness, Color color) {
    DrawLineEx(start, end, thickness, color);
    float dx = end.x - start.x, dy = end.y - start.y;
    float length = sqrtf(dx*dx + dy*dy);
    if (length == 0.0f) return;
    float ux = dx/length, uy = dy/length;
    float nodeRadius = 23.0f;
    Vector2 tip  = { end.x - ux*nodeRadius, end.y - uy*nodeRadius };
    float arrowLength = 18.0f, arrowWidth = 10.0f;
    Vector2 base = { tip.x - ux*arrowLength, tip.y - uy*arrowLength };
    float px = -uy, py = ux;
    Vector2 left  = { base.x + px*arrowWidth, base.y + py*arrowWidth };
    Vector2 right = { base.x - px*arrowWidth, base.y - py*arrowWidth };
    DrawLineEx(tip, left,  thickness+1.5f, color);
    DrawLineEx(tip, right, thickness+1.5f, color);
}

