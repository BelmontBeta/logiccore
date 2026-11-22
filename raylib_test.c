
#include "raylib.h"

int main(void)
{
    InitWindow(800, 450, "LOGIC CORE - Teste Raylib");
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawText("Raylib funcionando!", 250, 200, 30, DARKBLUE);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
