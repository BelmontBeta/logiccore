#include "display.h"
#include "raylib.h"

#include <stddef.h>

#define MAX_RESOLUTIONS 8

typedef struct {
    int width;
    int height;
} Resolution;

// Mesma proporcao do jogo (1100x760): sem barras pretas em modo janela.
// 1100x760 e a resolucao padrao e sempre fica disponivel.
static const Resolution presets[] = {
    {880, 608},
    {GAME_WIDTH, GAME_HEIGHT},
    {1320, 912},
    {1540, 1064},
    {1760, 1216}
};

#define PRESET_COUNT ((int)(sizeof(presets) / sizeof(presets[0])))

static Resolution available[MAX_RESOLUTIONS];
static int availableCount = 0;
static int currentIndex = 0;

static RenderTexture2D target;
static bool targetReady = false;

// Area da janela onde o jogo e desenhado (com barras pretas nas sobras).
static Rectangle destination = {0, 0, GAME_WIDTH, GAME_HEIGHT};

static void CenterWindow(int width, int height)
{
    int monitor = GetCurrentMonitor();
    Vector2 origin = GetMonitorPosition(monitor);

    SetWindowPosition(
        (int)origin.x + (GetMonitorWidth(monitor) - width) / 2,
        (int)origin.y + (GetMonitorHeight(monitor) - height) / 2
    );
}

static void ApplyWindowedSize(void)
{
    Resolution r = available[currentIndex];

    SetWindowSize(r.width, r.height);
    CenterWindow(r.width, r.height);
}

void Display_Init(void)
{
    int monitor = GetCurrentMonitor();
    int monitorWidth = GetMonitorWidth(monitor);
    int monitorHeight = GetMonitorHeight(monitor);

    availableCount = 0;
    currentIndex = 0;

    for (int i = 0; i < PRESET_COUNT && availableCount < MAX_RESOLUTIONS; i++) {

        bool isDefault = presets[i].width == GAME_WIDTH &&
                         presets[i].height == GAME_HEIGHT;

        // Deixa uma folga na altura para a barra de titulo e a barra de tarefas.
        bool fits = presets[i].width <= monitorWidth &&
                    presets[i].height <= monitorHeight - 80;

        if (isDefault)
            currentIndex = availableCount;

        if (isDefault || fits)
            available[availableCount++] = presets[i];
    }

    target = LoadRenderTexture(GAME_WIDTH, GAME_HEIGHT);
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);
    targetReady = target.id != 0;
}

void Display_Unload(void)
{
    if (targetReady) {
        UnloadRenderTexture(target);
        targetReady = false;
    }
}

void Display_Update(void)
{
    if (IsKeyPressed(KEY_F11))
        Display_ToggleFullscreen();

    float screenWidth = (float)GetScreenWidth();
    float screenHeight = (float)GetScreenHeight();

    float scaleX = screenWidth / GAME_WIDTH;
    float scaleY = screenHeight / GAME_HEIGHT;
    float scale = scaleX < scaleY ? scaleX : scaleY;

    destination.width = GAME_WIDTH * scale;
    destination.height = GAME_HEIGHT * scale;
    destination.x = (screenWidth - destination.width) / 2.0f;
    destination.y = (screenHeight - destination.height) / 2.0f;

    // O mouse passa a ser lido em coordenadas do jogo (1100x760), entao
    // menus, botoes e minigames continuam funcionando sem alteracoes.
    SetMouseOffset(-(int)destination.x, -(int)destination.y);
    SetMouseScale(1.0f / scale, 1.0f / scale);
}

void Display_BeginRender(void)
{
    BeginTextureMode(target);
    ClearBackground(BLACK);
}

void Display_EndRender(void)
{
    EndTextureMode();

    BeginDrawing();

    ClearBackground(BLACK);

    // A textura de um RenderTexture vem invertida no eixo Y.
    DrawTexturePro(
        target.texture,
        (Rectangle){0, 0, (float)GAME_WIDTH, -(float)GAME_HEIGHT},
        destination,
        (Vector2){0, 0},
        0.0f,
        WHITE
    );

    EndDrawing();
}

int Display_GetResolutionCount(void)
{
    return availableCount;
}

int Display_GetResolutionIndex(void)
{
    return currentIndex;
}

void Display_GetResolution(int index, int *width, int *height)
{
    if (index < 0 || index >= availableCount)
        index = 0;

    if (width != NULL)
        *width = available[index].width;

    if (height != NULL)
        *height = available[index].height;
}

void Display_SetResolution(int index)
{
    if (index < 0 || index >= availableCount)
        return;

    currentIndex = index;

    // Em tela cheia a escolha fica guardada e vale ao voltar para janela.
    if (!IsWindowFullscreen())
        ApplyWindowedSize();
}

bool Display_IsFullscreen(void)
{
    return IsWindowFullscreen();
}

void Display_ToggleFullscreen(void)
{
    if (!IsWindowFullscreen()) {

        int monitor = GetCurrentMonitor();

        SetWindowSize(
            GetMonitorWidth(monitor),
            GetMonitorHeight(monitor)
        );

        ToggleFullscreen();

    } else {

        ToggleFullscreen();
        ApplyWindowedSize();
    }
}