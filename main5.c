#include "raylib.h"
#include "animationui.h"
#include "animation.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <errno.h>
/* ── external declarations ── */
int dijkstra(Graph *g, int src, int dst, int *path);

/* ── constants ── */
#define SCREEN_W     900
#define SCREEN_H     650
#define MAX_TRAVELERS 16
#define NODE_TRAVEL_USEC 2000000

#define MAX_NODES 64
#define SEM_NAME_LEN 64
/* ── IPC message ── */
typedef enum {
    MSG_WAITING = 1,
    MSG_ENTERED = 2,
    MSG_FINISHED = 3
} MsgType;

typedef struct {
    MsgType type;
    int node;
    int nextNode;
} Msg;

/* ── per-traveler state kept by parent ── */
typedef struct {
    pid_t  pid;
    int    readFd;
    int    src, dst;
    int    curNode;
    bool   done;

    bool   waiting;
    int    waitingNode;

    Point  guiPos;

    int    fromNode;
    int    targetNode;
    float  moveStartTime;
    float  moveDuration;
    bool   moving;
} TravelerState;

/* ── colors ── */
static Color TRAVELER_COLORS[MAX_TRAVELERS] = {
    {220,  50,  50, 255}, {50,  200,  50, 255},
    { 50, 130, 255, 255}, {255, 200,   0, 255},
    {200,  50, 220, 255}, {  0, 220, 200, 255},
    {255, 130,   0, 255}, {255, 100, 180, 255},
    {100, 255, 130, 255}, {130, 180, 255, 255},
    {255,  80,  80, 255}, { 80, 255, 255, 255},
    {255, 255,  80, 255}, {180,  80, 255, 255},
    {255, 180,  80, 255}, { 80, 255, 180, 255},
};

/* ── helpers ── */
void DrawArrowLine(Vector2 start, Vector2 end, float thickness, Color color);


static void make_sem_name(char *buffer, int node) {
    snprintf(buffer, SEM_NAME_LEN, "/os_node_sem_%d", node);
}
/* ────────────────────────────────────────────────────────────────────────
 * CHILD PROCESS
 * Reads the graph itself, runs Dijkstra, travels, sends messages.
 * ──────────────────────────────────────────────────────────────────────── */
static void child_run(int writeFd, const char *filename, int src, int dst) {
    int n;
    Graph *g = loadGraph(filename, &n);
    if (!g) {
        close(writeFd);
        exit(1);
    }

    int path[64];
    int pathLen = dijkstra(g, src, dst, path);
    freeGraph(g);

    if (pathLen == 0) {
        close(writeFd);
        exit(1);
    }

    /* Travel: send a message for each node arrival */
    /* Travel: send a message for each node arrival */
    /* Travel with node synchronization */
    for (int i = 0; i < pathLen; i++) {
        int node = path[i];
        int next = (i + 1 < pathLen) ? path[i + 1] : -1;

        char semName[SEM_NAME_LEN];
        make_sem_name(semName, node);

        sem_t *nodeSem = sem_open(semName, 0);
        if (nodeSem == SEM_FAILED) {
            perror("child sem_open");
            close(writeFd);
            exit(1);
        }

        /* Tell parent: I am waiting outside this node */
        /* Try to enter the node immediately */
        if (sem_trywait(nodeSem) == -1) {
            if (errno == EAGAIN) {
                /* Node is busy: tell parent that this traveler is waiting */
                Msg waitingMsg = { MSG_WAITING, node, next };
                write(writeFd, &waitingMsg, sizeof(Msg));

                /* Now really wait until the node becomes free */
                sem_wait(nodeSem);
            } else {
                perror("sem_trywait");
                sem_close(nodeSem);
                close(writeFd);
                exit(1);
            }
        }

        /* Now the traveler is inside the node */
        Msg enteredMsg = { MSG_ENTERED, node, next };
        write(writeFd, &enteredMsg, sizeof(Msg));
        /* Stay inside the node for one full second */
        sleep(1);

        /* Leave the node */
        sem_post(nodeSem);
        sem_close(nodeSem);

        /* Travel on the edge to the next node */
        if (next != -1)
            usleep(NODE_TRAVEL_USEC);
    }

    /* Signal fully finished */
    Msg fin = { MSG_FINISHED, -2, -1 };
    write(writeFd, &fin, sizeof(Msg));
    close(writeFd);
    exit(0);
}

/* ────────────────────────────────────────────────────────────────────────
 * PARENT / MAIN
 * ──────────────────────────────────────────────────────────────────────── */
int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return 1;
    }
    const char *filename = argv[1];

    /* ── load graph + travelers ── */
    int n, numTravelers;
    int sources[MAX_TRAVELERS], dests[MAX_TRAVELERS];
    Graph *g = loadGraphAndTravelers(filename, &n, &numTravelers, sources, dests);
    if (!g) {
        fprintf(stderr, "Failed to load graph from %s\n", filename);
        return 1;
    }

    /* ── default node layout ── */
    Point pos[64];
    defaultLayout(n, pos);
    /* ── create one named semaphore per node ── */
    for (int i = 0; i < n; i++) {
        char semName[SEM_NAME_LEN];
        make_sem_name(semName, i);

        sem_unlink(semName);  /* remove old semaphore if it exists */

        sem_t *sem = sem_open(semName, O_CREAT | O_EXCL, 0600, 1);
        if (sem == SEM_FAILED) {
            perror("sem_open");
            freeGraph(g);
            return 1;
        }

        sem_close(sem);
    }
    /* ── create pipes and fork children ── */
    TravelerState travelers[MAX_TRAVELERS];
    int pipeFds[MAX_TRAVELERS][2]; /* [t][0]=read  [t][1]=write */

    for (int t = 0; t < numTravelers; t++) {
        if (pipe(pipeFds[t]) < 0) {
            perror("pipe");
            freeGraph(g);
            return 1;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            freeGraph(g);
            return 1;
        }

        if (pid == 0) {
            /* ── child ── */
            close(pipeFds[t][0]); /* close read end in child */
            /* close other travelers' pipe ends we inherited */
            for (int j = 0; j < t; j++) {
                close(pipeFds[j][0]);
                close(pipeFds[j][1]);
            }

            child_run(pipeFds[t][1], filename, sources[t], dests[t]);
            /* child_run never returns */
        }

        /* ── parent ── */
        close(pipeFds[t][1]); /* close write end in parent */

        /* make read end non-blocking so GUI loop doesn't stall */
        int flags = fcntl(pipeFds[t][0], F_GETFL, 0);
        fcntl(pipeFds[t][0], F_SETFL, flags | O_NONBLOCK);
      travelers[t].pid        = pid;
travelers[t].readFd     = pipeFds[t][0];
travelers[t].src        = sources[t];
travelers[t].dst        = dests[t];
travelers[t].curNode    = sources[t];
travelers[t].done       = false;

travelers[t].waiting     = false;
travelers[t].waitingNode = -1;
        travelers[t].waiting     = false;
        travelers[t].waitingNode = -1;

        travelers[t].guiPos     = pos[sources[t]];

        travelers[t].fromNode      = sources[t];
        travelers[t].targetNode    = sources[t];
        travelers[t].moveStartTime = 0.0f;
        travelers[t].moveDuration  = 1.5f;
        travelers[t].moving        = false;
    }

    /* ── raylib window ── */
    InitWindow(SCREEN_W, SCREEN_H, "Graph Simulation - Milestone 5 (IPC pipes)");
    SetTargetFPS(60);

    typedef enum { STATE_IDLE, STATE_RUNNING, STATE_FINISHED } AnimState;
    AnimState state = STATE_RUNNING; /* start immediately; children are already running */

    int doneCount = 0;

    /* GUI lerp state per traveler */
    Point guiPos[MAX_TRAVELERS];
    for (int t = 0; t < numTravelers; t++)
        guiPos[t] = pos[sources[t]];

    while (!WindowShouldClose()) {
        /* ── poll pipes for messages (non-blocking) ── */
        if (state == STATE_RUNNING) {
            for (int t = 0; t < numTravelers; t++) {
                if (travelers[t].done) continue;

                Msg m;
                ssize_t r = read(travelers[t].readFd, &m, sizeof(Msg));
                if (r == sizeof(Msg)) {
                    if (m.type == MSG_FINISHED) {
                        /* child finished */
                        printf("[PID=%d] finished\n", (int)travelers[t].pid);
                        fflush(stdout);

                        travelers[t].done = true;
                        travelers[t].waiting = false;
                        travelers[t].waitingNode = -1;

                        doneCount++;
                        close(travelers[t].readFd);

                    } else if (m.type == MSG_WAITING) {
                        /* child is waiting outside a node */
                        travelers[t].waiting = true;
                        travelers[t].waitingNode = m.node;

                    } else if (m.type == MSG_ENTERED) {
                        /* child entered the node - start smooth movement in GUI */
                        travelers[t].waiting = false;
                        travelers[t].waitingNode = -1;

                        if (travelers[t].curNode != m.node) {
                            travelers[t].fromNode = travelers[t].curNode;
                            travelers[t].targetNode = m.node;
                            travelers[t].moveStartTime = GetTime();
                            travelers[t].moveDuration = 1.5f;
                            travelers[t].moving = true;
                        } else {
                            guiPos[t] = pos[m.node];
                        }

                        travelers[t].curNode = m.node;

                        if (m.nextNode == -1) {
                            printf("[PID=%d] arrived at node %d | DESTINATION\n",
                                   (int)travelers[t].pid, m.node);
                        } else {
                            printf("[PID=%d] arrived at node %d | next node: %d\n",
                                   (int)travelers[t].pid, m.node, m.nextNode);
                        }

                        fflush(stdout);

                    }
                }
                /* EAGAIN / EWOULDBLOCK = no message yet, that's fine */
            }
        }

        //* ── update smooth GUI movement ── */
        for (int t = 0; t < numTravelers; t++) {
            if (travelers[t].moving) {
                float elapsed = GetTime() - travelers[t].moveStartTime;
                float alpha = elapsed / travelers[t].moveDuration;

                if (alpha >= 1.0f) {
                    alpha = 1.0f;
                    travelers[t].moving = false;
                }

                Point a = pos[travelers[t].fromNode];
                Point b = pos[travelers[t].targetNode];

                guiPos[t].x = a.x + (b.x - a.x) * alpha;
                guiPos[t].y = a.y + (b.y - a.y) * alpha;
            }
        }
        if (state == STATE_RUNNING && doneCount == numTravelers) {
            bool allAtDestinations = true;
            bool anyMoving = false;

            for (int t = 0; t < numTravelers; t++) {
                if (travelers[t].curNode != travelers[t].dst) {
                    allAtDestinations = false;
                }

                if (travelers[t].moving) {
                    anyMoving = true;
                }
            }

            if (allAtDestinations && !anyMoving) {
                state = STATE_FINISHED;
            }
        }
        /* ── draw ── */
        BeginDrawing();
        ClearBackground((Color){ 20, 30, 45, 255 });

        /* graph edges */
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
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
                    DrawRectangle(midX+ox-tw/2-5, midY+oy-fs/2-3, tw+10, fs+6,
                                  Fade(BLACK, 0.75f));
                    DrawText(wt, midX+ox-tw/2, midY+oy-fs/2, fs, YELLOW);
                }
            }
        }

        /* nodes */
        for (int i = 0; i < n; i++) {
            DrawCircle(pos[i].x, pos[i].y, 20, RAYWHITE);
            DrawCircleLines(pos[i].x, pos[i].y, 15, LIGHTGRAY);
            DrawText(TextFormat("%d", i), pos[i].x - 6, pos[i].y - 8, 18, BLACK);
        }

        /* travelers (dots at last reported node) */
        /* travelers */
        for (int t = 0; t < numTravelers; t++) {
            Color col = TRAVELER_COLORS[t % MAX_TRAVELERS];
            Vector2 ap = { guiPos[t].x, guiPos[t].y };

            if (travelers[t].waiting) {
                Point nodePos = pos[travelers[t].waitingNode];

                float offsetX = 28.0f + (t % 3) * 10.0f;
                float offsetY = -28.0f - (t % 3) * 10.0f;

                ap.x = nodePos.x + offsetX;
                ap.y = nodePos.y + offsetY;

                DrawCircleV(ap, 22, Fade(ORANGE, 0.30f));
                DrawCircleV(ap, 13, ORANGE);
                DrawCircleLinesV(ap, 15, WHITE);
                DrawText("WAIT", ap.x + 15, ap.y - 8, 12, ORANGE);
            } else {
                DrawCircleV(ap, 20, Fade(col, 0.25f));
                DrawCircleV(ap, 13, col);
                DrawCircleLinesV(ap, 15, WHITE);
            }

            DrawText(TextFormat("%d", t), ap.x - 4, ap.y - 8, 14, BLACK);
        }

        /* legend */
        for (int t = 0; t < numTravelers; t++) {
            Color col = TRAVELER_COLORS[t % MAX_TRAVELERS];
            int legendY = 25 + t * 28;
            DrawCircle(SCREEN_W - 130, legendY + 10, 10, col);
            DrawText(TextFormat("T%d: %d->%d [PID:%d]%s",
                                t, travelers[t].src, travelers[t].dst,
                                (int)travelers[t].pid,
                                travelers[t].done ? " DONE" : ""),
                     SCREEN_W - 115, legendY, 14, col);
        }

        /* status */
        if (state == STATE_RUNNING)
            DrawText("STATUS: RUNNING", 25, 25, 18, LIME);
        else if (state == STATE_FINISHED) {
            DrawText("STATUS: FINISHED", 25, 25, 18, SKYBLUE);
            DrawText("All Travelers Reached Destination!", 200, 590, 22, RAYWHITE);
        }

        EndDrawing();
    }

    /* ── cleanup ── */
    for (int t = 0; t < numTravelers; t++) {
        if (!travelers[t].done) {
            kill(travelers[t].pid, SIGTERM);
            close(travelers[t].readFd);
        }
        waitpid(travelers[t].pid, NULL, 0);
    }

    freeGraph(g);
    CloseWindow();
    return 0;
}

/* ── DrawArrowLine (unchanged from milestone 4) ── */
void DrawArrowLine(Vector2 start, Vector2 end, float thickness, Color color) {
    DrawLineEx(start, end, thickness, color);
    float dx = end.x - start.x, dy = end.y - start.y;
    float length = sqrtf(dx*dx + dy*dy);
    if (length == 0.0f) return;
    float ux = dx/length, uy = dy/length;
    float nodeRadius = 23.0f;
    Vector2 tip  = { end.x - ux*nodeRadius, end.y - uy*nodeRadius };
    float arrowLength = 18.0f, arrowWidth = 10.0f;
    Vector2 base  = { tip.x - ux*arrowLength, tip.y - uy*arrowLength };
    float px = -uy, py = ux;
    Vector2 left  = { base.x + px*arrowWidth, base.y + py*arrowWidth };
    Vector2 right = { base.x - px*arrowWidth, base.y - py*arrowWidth };
    DrawLineEx(tip, left,  thickness+1.5f, color);
    DrawLineEx(tip, right, thickness+1.5f, color);
}
