
#include "raylib.h"
#include "core/game.h"

int main(void)
{
    InitWindow(1100, 760, "LOGIC CORE");
    SetTargetFPS(60);

    Game_Init();

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        Game_Update(dt);

        BeginDrawing();
        Game_Draw();
        EndDrawing();
    }

    Game_Unload();
    CloseWindow();

    return 0;
}
