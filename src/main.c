#include "raylib.h"
#include "core/game.h"
#include "systems/display.h"
#include "systems/music.h"

int main(void)
{
    InitWindow(GAME_WIDTH, GAME_HEIGHT, "LOGIC CORE");

    // ESC deixa de fechar a janela: agora abre o menu de pausa.
    SetExitKey(KEY_NULL);

    Display_Init();
    InitGameMusic();
    SetTargetFPS(60);

    Game_Init();

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        Display_Update();
        UpdateGameMusic();
        Game_Update(dt);

        // O jogo e desenhado em 1100x760 e escalado para a janela.
        Display_BeginRender();
        Game_Draw();
        Display_EndRender();
    }

    Game_Unload();
    CloseGameMusic();
    Display_Unload();
    CloseWindow();

    return 0;
}
