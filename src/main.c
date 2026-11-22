#include "raylib.h"
#include "core/game.h"
#include "systems/music.h"

int main(void)
{
    InitWindow(1100, 760, "LOGIC CORE");
    InitGameMusic();
    SetTargetFPS(60);

    Game_Init();

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        UpdateGameMusic();
        Game_Update(dt);

        BeginDrawing();
        Game_Draw();
        EndDrawing();
    }

    Game_Unload();
    CloseGameMusic();
    CloseWindow();

    return 0;
}