
#include "menu.h"
#include "raylib.h"

#define SCREEN_WIDTH 1100
#define SCREEN_HEIGHT 760

static Texture2D background;
static bool menuReady = false;

static Rectangle startButton = {
    390, 570, 320, 75
};

bool Menu_Init(void)
{
    background = LoadTexture(
        "assets/sprites/history/menu.png"
    );

    if (background.id == 0) {
        TraceLog(LOG_ERROR, "Erro ao carregar menu.png");
        return false;
    }

    menuReady = true;
    return true;
}

bool Menu_Update(void)
{
    if (!menuReady)
        return false;

    Vector2 mouse = GetMousePosition();

    bool hovering = CheckCollisionPointRec(
        mouse,
        startButton
    );

    return hovering &&
           IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

void Menu_Draw(void)
{
    ClearBackground(BLACK);

    if (!menuReady)
        return;

    // Fundo.
    float scaleX = (float)SCREEN_WIDTH / background.width;
    float scaleY = (float)SCREEN_HEIGHT / background.height;

    float scale = scaleX < scaleY ? scaleX : scaleY;

    float width = background.width * scale;
    float height = background.height * scale;

    DrawTextureEx(
        background,
        (Vector2){
            (SCREEN_WIDTH - width) / 2.0f,
            (SCREEN_HEIGHT - height) / 2.0f
        },
        0.0f,
        scale,
        WHITE
    );

    // Titulo.
    const char *title = "LOGIC CORE";

    int titleWidth = MeasureText(title, 68);

    DrawText(
        title,
        (SCREEN_WIDTH - titleWidth) / 2 + 3,
        116,
        68,
        (Color){0, 0, 0, 190}
    );

    DrawText(
        title,
        (SCREEN_WIDTH - titleWidth) / 2,
        113,
        68,
        SKYBLUE
    );

    // Subtitulo.
    const char *subtitle = "Um desafio logico";

    int subtitleWidth = MeasureText(subtitle, 24);

    int subtitleX = (SCREEN_WIDTH - subtitleWidth) / 2;

    DrawText(
        subtitle,
        subtitleX + 2,
        201,
        24,
        (Color){0, 0, 0, 220}
    );

    DrawText(
        subtitle,
        subtitleX,
        199,
        24,
        WHITE
    );

    // Botao.
    Vector2 mouse = GetMousePosition();

    bool hovering = CheckCollisionPointRec(
        mouse,
        startButton
    );

    Color buttonColor = hovering
        ? (Color){0, 145, 205, 250}
        : (Color){10, 55, 95, 235};

    Color borderColor = hovering
        ? (Color){100, 235, 255, 255}
        : SKYBLUE;

    if (hovering) {
        DrawRectangleRounded(
            (Rectangle){
                startButton.x - 5,
                startButton.y - 5,
                startButton.width + 10,
                startButton.height + 10
            },
            0.2f,
            8,
            (Color){0, 180, 255, 45}
        );
    }

    DrawRectangleRounded(
        startButton,
        0.2f,
        8,
        buttonColor
    );

    DrawRectangleRoundedLinesEx(
        startButton,
        0.2f,
        8,
        3,
        borderColor
    );

    const char *buttonText = "INICIAR JOGO";

    int buttonTextWidth = MeasureText(buttonText, 28);

    DrawText(
        buttonText,
        (SCREEN_WIDTH - buttonTextWidth) / 2,
        592,
        28,
        WHITE
    );
}

void Menu_Unload(void)
{
    if (background.id != 0) {
        UnloadTexture(background);
        background.id = 0;
    }

    menuReady = false;
}
