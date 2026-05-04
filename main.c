#include "raylib.h"
#include "animationui.h"
#include "animation.h"
#include <stdio.h>

#include <math.h>

int dijkstra(Graph *g, int src, int dst, int *path);
#define SCREEN_W 900
#define SCREEN_H 650

typedef enum {
    STATE_IDLE,
    STATE_RUNNING,
    STATE_PAUSED,
    STATE_FINISHED
} AnimState;

void DrawArrowLine(Vector2 start, Vector2 end, float thickness, Color color);

int main(int argc, char **argv) {
    if (argc < 2) return 1;

    int n;
    Graph *g = loadGraph(argv[1], &n);
    if (!g) return 1;

    int path[64];
    int len = dijkstra(g, 0, n - 1, path);

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
                if (g->weights[i][j] > 0 && i < j) {

                    Vector2 start = { pos[i].x, pos[i].y };
                    Vector2 end = { pos[j].x, pos[j].y };

                    // Draw the normal edge
                    DrawArrowLine(start, end, 1.5f, Fade(LIGHTGRAY, 0.5f));

                    // Calculate edge direction
                    float dx = end.x - start.x;
                    float dy = end.y - start.y;
                    float length = sqrtf(dx * dx + dy * dy);

                    // Calculate the middle of the edge
                    float midX = (start.x + end.x) / 2.0f;
                    float midY = (start.y + end.y) / 2.0f;

                    // Move the weight slightly away from the edge
                    float offsetX = 0.0f;
                    float offsetY = 0.0f;

                    if (length != 0.0f) {
                        offsetX = -dy / length * 22.0f;
                        offsetY =  dx / length * 22.0f;
                    }

                    // Prepare weight text
                    const char *weightText = TextFormat("%d", g->weights[i][j]);
                    int fontSize = 18;
                    int textWidth = MeasureText(weightText, fontSize);

                    float textX = midX + offsetX - textWidth / 2.0f;
                    float textY = midY + offsetY - fontSize / 2.0f;

                    // Draw dark background behind the weight
                    DrawRectangle(
                        textX - 5,
                        textY - 3,
                        textWidth + 10,
                        fontSize + 6,
                        Fade(BLACK, 0.75f)
                    );

                    // Draw the weight
                    DrawText(weightText, textX, textY, fontSize, YELLOW);
                }
            }
        }
        for (int k = 0; k < len - 1; k++) {
            int from = path[k];
            int to = path[k + 1];

            Vector2 start = { pos[from].x, pos[from].y };
            Vector2 end = { pos[to].x, pos[to].y };

            DrawArrowLine(start, end, 8.0f, DARKBLUE);
            DrawArrowLine(start, end, 5.0f, SKYBLUE);
        }

        for (int i = 0; i < n; i++) {
            DrawCircle(pos[i].x, pos[i].y, 20, RAYWHITE);
            DrawCircleLines(pos[i].x, pos[i].y, 15, LIGHTGRAY);
            DrawText(TextFormat("%d", i), pos[i].x - 6, pos[i].y - 8, 18, BLACK);
        }

        Vector2 agentPos = { agent.x, agent.y };

        DrawCircleV(agentPos, 20, Fade(PURPLE, 0.25f));
        DrawCircleV(agentPos, 13, MAROON);
        DrawCircleLinesV(agentPos, 15, WHITE);

        if (state == STATE_RUNNING) DrawText("STATUS: RUNNING", 160, 35, 18, LIME);
        else if (state == STATE_PAUSED)  DrawText("STATUS: PAUSED", 160, 35, 18, ORANGE);
        else if (state == STATE_FINISHED) {
            DrawText("STATUS: FINISHED", 160, 35, 18, SKYBLUE);
            DrawText("Destination Reached!", 320, 585, 28, RAYWHITE);
        }
        else DrawText("STATUS: READY", 160, 35, 18, LIGHTGRAY);

        EndDrawing();
    }

    freeAnimationPath(anim);
    freeGraph(g);
    CloseWindow();
    return 0;

}
void DrawArrowLine(Vector2 start, Vector2 end, float thickness, Color color)
{
    // Draw the main line
    DrawLineEx(start, end, thickness, color);

    float dx = end.x - start.x;
    float dy = end.y - start.y;
    float length = sqrtf(dx * dx + dy * dy);

    if (length == 0.0f) {
        return;
    }

    // Direction from start to end
    float ux = dx / length;
    float uy = dy / length;

    // Move arrow head away from the node center
    float nodeRadius = 23.0f;

    Vector2 tip = {
        end.x - ux * nodeRadius,
        end.y - uy * nodeRadius
    };

    // Arrow size
    float arrowLength = 18.0f;
    float arrowWidth = 10.0f;

    // Base point behind the tip
    Vector2 base = {
        tip.x - ux * arrowLength,
        tip.y - uy * arrowLength
    };

    // Perpendicular direction
    float px = -uy;
    float py = ux;

    Vector2 left = {
        base.x + px * arrowWidth,
        base.y + py * arrowWidth
    };

    Vector2 right = {
        base.x - px * arrowWidth,
        base.y - py * arrowWidth
    };

    // Draw arrow head using two lines
    DrawLineEx(tip, left, thickness + 1.5f, color);
    DrawLineEx(tip, right, thickness + 1.5f, color);
}
