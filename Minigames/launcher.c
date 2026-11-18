#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_W 900
#define SCREEN_H 620

/* Nomes dos executáveis na mesma pasta do launcher.exe. */
#define GAME_BIT      "doodle_logic.exe"
#define GAME_TERMINAL "terminal_simplificacao.exe"
#define GAME_CIPHER   "cipher_logic.exe"

typedef struct {
    Rectangle bounds;
    const char *title;
    const char *description;
    const char *executable;
    Color accent;
} MenuButton;

static bool LaunchGame(const char *executable)
{
    char command[512];

    /*
     * "start" abre o jogo em outro processo.
     * As aspas vazias após start definem o título da janela do comando.
     */
    snprintf(
        command,
        sizeof(command),
        "start \"\" \"%s\"",
        executable
    );

    int result = system(command);

    if (result != 0) {
    fprintf(
        stderr,
        "Nao foi possivel iniciar: %s\n"
        "Confira se o executavel esta na mesma pasta do launcher.exe.\n",
        executable
    );

    return false;
}

    return true;
}

static bool IsMouseOver(Rectangle bounds)
{
    return CheckCollisionPointRec(GetMousePosition(), bounds);
}

static bool DrawMenuButton(const MenuButton *button)
{
    bool hover = IsMouseOver(button->bounds);

    Color background = hover
        ? (Color){39, 55, 78, 255}
        : (Color){24, 36, 54, 255};

    DrawRectangleRounded(button->bounds, 0.12f, 8, background);

    DrawRectangleRoundedLinesEx(
        button->bounds,
        0.12f,
        8,
        hover ? 2.5f : 1.5f,
        button->accent
    );

    DrawCircle(
        (int)(button->bounds.x + 28),
        (int)(button->bounds.y + button->bounds.height / 2),
        7,
        button->accent
    );

    DrawText(
        button->title,
        (int)button->bounds.x + 50,
        (int)button->bounds.y + 12,
        22,
        RAYWHITE
    );

    DrawText(
        button->description,
        (int)button->bounds.x + 50,
        (int)button->bounds.y + 43,
        15,
        (Color){175, 190, 210, 255}
    );

    return hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

int main(void)
{
    InitWindow(
        SCREEN_W,
        SCREEN_H,
        "Logic Core - Selecao de Minigames"
    );

    SetTargetFPS(60);

    const MenuButton buttons[3] = {
        {
            {175, 190, 550, 82},
            "A Fuga de BIT",
            "Fusione formulas e supere as travas logicas.",
            GAME_BIT,
            (Color){64, 220, 230, 255}
        },
        {
            {175, 292, 550, 82},
            "Terminal de Simplificacao",
            "Escolha regras para simplificar expressoes logicas.",
            GAME_TERMINAL,
            (Color){255, 200, 96, 255}
        },
        {
            {175, 394, 550, 82},
            "Cipher Logic",
            "Monte formulas logicas para traduzir frases.",
            GAME_CIPHER,
            (Color){181, 137, 255, 255}
        }
    };

    while (!WindowShouldClose()) {
        int gameToLaunch = -1;

        if (IsKeyPressed(KEY_ONE))
            gameToLaunch = 0;
        else if (IsKeyPressed(KEY_TWO))
            gameToLaunch = 1;
        else if (IsKeyPressed(KEY_THREE))
            gameToLaunch = 2;

        BeginDrawing();
        ClearBackground((Color){10, 15, 25, 255});

        DrawRectangle(
            0,
            0,
            SCREEN_W,
            8,
            (Color){64, 220, 230, 255}
        );

        const char *title = "LOGIC CORE";
        int titleWidth = MeasureText(title, 42);

        DrawText(
            title,
            (SCREEN_W - titleWidth) / 2,
            54,
            42,
            (Color){64, 220, 230, 255}
        );

        const char *subtitle = "SELECIONE UM MINIGAME";
        int subtitleWidth = MeasureText(subtitle, 18);

        DrawText(
            subtitle,
            (SCREEN_W - subtitleWidth) / 2,
            112,
            18,
            (Color){185, 198, 216, 255}
        );

        for (int i = 0; i < 3; i++) {
            if (DrawMenuButton(&buttons[i]))
                gameToLaunch = i;
        }

        const char *shortcuts =
            "Atalhos: 1, 2 ou 3 para abrir um jogo";

        int shortcutsWidth = MeasureText(shortcuts, 15);

        DrawText(
            shortcuts,
            (SCREEN_W - shortcutsWidth) / 2,
            520,
            15,
            (Color){145, 165, 190, 255}
        );

        const char *footer = "ESC ou X para sair";
        int footerWidth = MeasureText(footer, 14);

        DrawText(
            footer,
            (SCREEN_W - footerWidth) / 2,
            565,
            14,
            (Color){125, 145, 170, 255}
        );

        EndDrawing();

        if (gameToLaunch >= 0)
            LaunchGame(buttons[gameToLaunch].executable);
    }

    CloseWindow();
    return 0;
}