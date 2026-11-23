#include "options.h"
#include "raylib.h"

#include "../systems/display.h"
#include "../systems/music.h"

#include <stdbool.h>

static OptionsContext context = OPTIONS_FROM_MENU;
static bool draggingVolume = false;
static bool confirmMainMenu = false;

// Layout (coordenadas do jogo, 1100x760).
static const Rectangle panel = {250, 120, 600, 520};

static const Rectangle resolutionLeft  = {500, 240, 46, 46};
static const Rectangle resolutionRight = {774, 240, 46, 46};

static const Rectangle fullscreenButton = {500, 310, 320, 46};

static const Rectangle volumeTrack = {500, 397, 250, 12};
static const Rectangle volumeHitbox = {488, 380, 274, 46};

static const Rectangle primaryButton   = {375, 470, 350, 56};
static const Rectangle secondaryButton = {375, 545, 350, 56};

static bool Hover(Rectangle bounds)
{
    return CheckCollisionPointRec(GetMousePosition(), bounds);
}

static void DrawButton(
    Rectangle bounds,
    const char *text,
    int fontSize,
    bool enabled,
    Color textColor
)
{
    bool hovering = enabled && Hover(bounds);

    Color fill = !enabled
        ? (Color){25, 35, 50, 235}
        : (hovering ? (Color){0, 145, 205, 250}
                    : (Color){10, 55, 95, 235});

    Color border = !enabled
        ? GRAY
        : (hovering ? (Color){100, 235, 255, 255} : SKYBLUE);

    DrawRectangleRounded(bounds, 0.2f, 8, fill);
    DrawRectangleRoundedLinesEx(bounds, 0.2f, 8, 2, border);

    int textWidth = MeasureText(text, fontSize);

    DrawText(
        text,
        (int)(bounds.x + (bounds.width - textWidth) / 2),
        (int)(bounds.y + (bounds.height - fontSize) / 2),
        fontSize,
        enabled ? textColor : GRAY
    );
}

static void ChangeResolution(int step)
{
    int count = Display_GetResolutionCount();

    if (count <= 1)
        return;

    int index = (Display_GetResolutionIndex() + step + count) % count;

    Display_SetResolution(index);
}

static void UpdateVolumeFromMouse(void)
{
    float value =
        (GetMousePosition().x - volumeTrack.x) / volumeTrack.width;

    SetGameVolume(value);
}

void Options_Open(OptionsContext newContext)
{
    context = newContext;
    draggingVolume = false;
    confirmMainMenu = false;
}

OptionsAction Options_Update(void)
{
    bool click = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    // ESC fecha o painel: continua o jogo ou volta ao menu.
    if (IsKeyPressed(KEY_ESCAPE)) {
        draggingVolume = false;
        confirmMainMenu = false;

        return context == OPTIONS_FROM_PAUSE
            ? OPTIONS_RESUME
            : OPTIONS_CLOSE;
    }

    // Volume (arrastar o controle).
    if (click && Hover(volumeHitbox))
        draggingVolume = true;

    if (draggingVolume) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            UpdateVolumeFromMouse();
        else
            draggingVolume = false;
    }

    // Clique fora do botao "menu inicial" cancela a confirmacao.
    if (click && !(context == OPTIONS_FROM_PAUSE &&
                   Hover(secondaryButton))) {
        confirmMainMenu = false;
    }

    if (!click)
        return OPTIONS_NONE;

    // Resolucao (somente em janela).
    if (!Display_IsFullscreen()) {
        if (Hover(resolutionLeft))
            ChangeResolution(-1);

        if (Hover(resolutionRight))
            ChangeResolution(1);
    }

    // Tela cheia.
    if (Hover(fullscreenButton))
        Display_ToggleFullscreen();

    // Botoes de baixo.
    if (Hover(primaryButton)) {
        draggingVolume = false;

        return context == OPTIONS_FROM_PAUSE
            ? OPTIONS_RESUME
            : OPTIONS_CLOSE;
    }

    if (context == OPTIONS_FROM_PAUSE && Hover(secondaryButton)) {

        if (confirmMainMenu) {
            confirmMainMenu = false;
            return OPTIONS_MAIN_MENU;
        }

        confirmMainMenu = true;
    }

    return OPTIONS_NONE;
}

void Options_Draw(void)
{
    bool fullscreen = Display_IsFullscreen();
    bool pause = context == OPTIONS_FROM_PAUSE;

    // Escurece o que esta atras do painel.
    DrawRectangle(0, 0, GAME_WIDTH, GAME_HEIGHT, (Color){0, 0, 0, 170});

    DrawRectangleRounded(panel, 0.05f, 8, (Color){8, 20, 38, 245});
    DrawRectangleRoundedLinesEx(panel, 0.05f, 8, 3, SKYBLUE);

    // Titulo.
    const char *title = pause ? "PAUSA" : "OPCOES";
    int titleWidth = MeasureText(title, 40);

    DrawText(
        title,
        (int)(panel.x + (panel.width - titleWidth) / 2),
        148,
        40,
        SKYBLUE
    );

    // Resolucao.
    DrawText("RESOLUCAO", 290, 252, 22, WHITE);

    int width = 0;
    int height = 0;
    Display_GetResolution(Display_GetResolutionIndex(), &width, &height);

    bool canChangeResolution =
        !fullscreen && Display_GetResolutionCount() > 1;

    DrawButton(resolutionLeft, "<", 28, canChangeResolution, WHITE);
    DrawButton(resolutionRight, ">", 28, canChangeResolution, WHITE);

    const char *resolutionText = TextFormat("%d x %d", width, height);
    int resolutionWidth = MeasureText(resolutionText, 24);

    DrawText(
        resolutionText,
        (int)(resolutionLeft.x + resolutionLeft.width +
              (resolutionRight.x - resolutionLeft.x -
               resolutionLeft.width - resolutionWidth) / 2),
        252,
        24,
        fullscreen ? GRAY : WHITE
    );

    if (fullscreen) {
        DrawText(
            "Desative a tela cheia para mudar a resolucao.",
            500, 292, 14, LIGHTGRAY
        );
    }

    // Tela cheia.
    DrawText("TELA CHEIA (F11)", 290, 322, 22, WHITE);

    DrawButton(
        fullscreenButton,
        fullscreen ? "LIGADA" : "DESLIGADA",
        24,
        true,
        fullscreen ? GREEN : WHITE
    );

    // Volume.
    DrawText("VOLUME", 290, 392, 22, WHITE);

    float volume = GetGameVolume();

    DrawRectangleRounded(
        volumeTrack, 1.0f, 6, (Color){40, 55, 75, 255}
    );

    DrawRectangleRounded(
        (Rectangle){
            volumeTrack.x,
            volumeTrack.y,
            volumeTrack.width * volume,
            volumeTrack.height
        },
        1.0f, 6, SKYBLUE
    );

    bool knobActive = draggingVolume || Hover(volumeHitbox);

    DrawCircle(
        (int)(volumeTrack.x + volumeTrack.width * volume),
        (int)(volumeTrack.y + volumeTrack.height / 2),
        knobActive ? 13.0f : 11.0f,
        knobActive ? (Color){100, 235, 255, 255} : WHITE
    );

    DrawText(
        TextFormat("%d%%", (int)(volume * 100.0f + 0.5f)),
        770, 392, 22, WHITE
    );

    // Botoes de baixo.
    DrawButton(
        primaryButton,
        pause ? "CONTINUAR" : "VOLTAR",
        26,
        true,
        WHITE
    );

    if (pause) {
        DrawButton(
            secondaryButton,
            confirmMainMenu
                ? "CONFIRMAR? PROGRESSO SERA PERDIDO"
                : "VOLTAR AO MENU INICIAL",
            confirmMainMenu ? 18 : 22,
            true,
            confirmMainMenu ? (Color){255, 120, 120, 255} : WHITE
        );
    }

    DrawText(
        pause ? "ESC: continuar" : "ESC: voltar",
        (int)(panel.x + 24),
        (int)(panel.y + panel.height - 34),
        16,
        LIGHTGRAY
    );
}
