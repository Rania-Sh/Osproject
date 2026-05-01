#include "raylib.h"
#include "animationui.h"
#include "animation.h"
#include <stdio.h>

#define SCREEN_W 900
#define SCREEN_H 650

typedef enum {
    STATE_IDLE,
    STATE_RUNNING,
    STATE_PAUSED,
    STATE_FINISHED
} AnimState;

int main(int argc, char **argv) {
    if (argc < 2) return 1;

    int n;
    Graph *g = loadGraph(argv[1], &n);
    if (!g) return 1;

    int path[64];
    int len = findShortestPath(g, 0, n - 1, path);

    if (len == 0) {
        freeGraph(g);
        return 1;
    }

    Point pos[64];
    defaultLayout(n, pos);

    AnimationPath *anim = buildAnimationPath(g, path, len, pos);

    InitWindow(SCREEN_W, SCREEN_H, "Graph Animation System");
    SetTargetFPS(60);

    AnimState state = STATE_IDLE;
    Point agent = pos[path[0]];

    Rectangle btnAction = { 25, 25, 120, 40 };
    Rectangle btnRestart = { 25, 75, 120, 40 };

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        Vector2 mousePos = GetMousePosition();

        bool hoverAction = CheckCollisionPointRec(mousePos, btnAction);
        bool hoverRestart = CheckCollisionPointRec(mousePos, btnRestart);

        if (state == STATE_RUNNING) {
            if (drawButton(btnAction, "Stop", ORANGE, WHITE, hoverAction)) {
                state = STATE_PAUSED;
            }
        } else {
            if (drawButton(btnAction, "Play", LIME, DARKGRAY, hoverAction)) {
                if (state == STATE_FINISHED) {
                    anim->currentStep = 0;
                    anim->elapsed = 0.0f;
                    anim->finished = false;
                    agent = pos[path[0]];
                }
                state = STATE_RUNNING;
            }
        }

        if (drawButton(btnRestart, "Restart", MAROON, WHITE, hoverRestart)) {
            state = STATE_IDLE;
            anim->currentStep = 0;
            anim->elapsed = 0.0f;
            anim->finished = false;
            agent = pos[path[0]];
        }

        if (state == STATE_RUNNING) {
            updateAnimation(anim, dt, &agent);
            if (anim->finished) state = STATE_FINISHED;
        }

        BeginDrawing();
        ClearBackground((Color){ 20, 30, 45, 255 });

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (g->weights[i][j] > 0) {
                    DrawLineEx((Vector2){pos[i].x, pos[i].y}, (Vector2){pos[j].x, pos[j].y}, 1.5f, Fade(LIGHTGRAY, 0.5f));
                }
            }
        }

        for (int i = 0; i < n; i++) {
            DrawCircle(pos[i].x, pos[i].y, 20, RAYWHITE);
            DrawCircleLines(pos[i].x, pos[i].y, 20, LIGHTGRAY);
            DrawText(TextFormat("%d", i), pos[i].x - 6, pos[i].y - 8, 18, BLACK);
        }

        DrawCircleV((Vector2){agent.x, agent.y}, 14, YELLOW);
        DrawCircleLinesV((Vector2){agent.x, agent.y}, 14, ORANGE);

        if (state == STATE_RUNNING) DrawText("STATUS: RUNNING", 160, 35, 18, LIME);
        else if (state == STATE_PAUSED)  DrawText("STATUS: PAUSED", 160, 35, 18, ORANGE);
        else if (state == STATE_FINISHED) DrawText("STATUS: FINISHED", 160, 35, 18, SKYBLUE);
        else DrawText("STATUS: READY", 160, 35, 18, LIGHTGRAY);

        EndDrawing();
    }

    freeAnimationPath(anim);
    freeGraph(g);
    CloseWindow();
    return 0;
}
