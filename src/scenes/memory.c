
#include "memory.h"
#include "raylib.h"

#include <math.h>

#define SCREEN_WIDTH 1100
#define SCREEN_HEIGHT 760
#define TOTAL_SCENES 8

static Texture2D scenes[TOTAL_SCENES];

static int currentScenes[3];
static int sceneCount = 0;
static int currentIndex = 0;

static float zoom = 1.0f;
static float zoomTimer = 0.0f;

#define ZOOM_AMPLITUDE 0.035f
#define ZOOM_FREQUENCY 0.65f

static bool memoryActive = false;

bool Memory_Init(void)
{
    const char *paths[TOTAL_SCENES] = {
        "assets/sprites/history/cena1.png",
        "assets/sprites/history/cena2.png",
        "assets/sprites/history/cena3.png",
        "assets/sprites/history/cena4.png",
        "assets/sprites/history/cena5.png",
        "assets/sprites/history/cena6.png",
        "assets/sprites/history/cena7.png",
        "assets/sprites/history/cena8.png"
    };

    bool success = true;

    for (int i = 0; i < TOTAL_SCENES; i++) {
        scenes[i] = LoadTexture(paths[i]);

        if (scenes[i].id == 0) {
            TraceLog(LOG_ERROR, "Erro ao carregar: %s", paths[i]);
            success = false;
        }
    }

    return success;
}

void Memory_Start(MemorySequence sequence)
{
    currentIndex = 0;
    zoom = 1.0f;
    zoomTimer = 0.0f;
    sceneCount = 0;

    switch (sequence) {

        case MEMORY_INTRO:
            currentScenes[0] = 0;
            currentScenes[1] = 1;
            currentScenes[2] = 2;
            sceneCount = 3;
            break;

        case MEMORY_BETWEEN_ROOMS:
            currentScenes[0] = 7;
            sceneCount = 1;
            break;

        case MEMORY_ROOM2:
            currentScenes[0] = 3;
            sceneCount = 1;
            break;

        case MEMORY_DISCOVERY:
            currentScenes[0] = 4;
            currentScenes[1] = 5;
            sceneCount = 2;
            break;

        case MEMORY_ENDING:
            currentScenes[0] = 6;
            sceneCount = 1;
            break;

        default:
            break;
    }

    memoryActive = sceneCount > 0;
}

bool Memory_Update(float dt)
{
    if (!memoryActive)
        return true;

    // Zoom suave e continuo: aproximacao e afastamento.
    zoomTimer += dt;

    zoom = 1.0f +
        (1.0f - cosf(zoomTimer * ZOOM_FREQUENCY)) *
        (ZOOM_AMPLITUDE / 2.0f);

    if (IsKeyPressed(KEY_ENTER) ||
        IsKeyPressed(KEY_SPACE)) {

        currentIndex++;
        zoom = 1.0f;
        zoomTimer = 0.0f;

        if (currentIndex >= sceneCount) {
            memoryActive = false;
            return true;
        }
    }

    return false;
}

void Memory_Draw(void)
{
    ClearBackground(BLACK);

    if (!memoryActive || sceneCount == 0)
        return;

    int sceneIndex = currentScenes[currentIndex];

    Texture2D texture = scenes[sceneIndex];

    if (texture.id == 0)
        return;

    float scaleX = (float)SCREEN_WIDTH / texture.width;
    float scaleY = (float)SCREEN_HEIGHT / texture.height;

    float baseScale = scaleX < scaleY ? scaleX : scaleY;

    float width = texture.width * baseScale * zoom;
    float height = texture.height * baseScale * zoom;

    Rectangle source = {
        0,
        0,
        (float)texture.width,
        (float)texture.height
    };

    Rectangle destination = {
        (SCREEN_WIDTH - width) / 2.0f,
        (SCREEN_HEIGHT - height) / 2.0f,
        width,
        height
    };

    Vector2 origin = {0, 0};

    DrawTexturePro(
        texture,
        source,
        destination,
        origin,
        0.0f,
        WHITE
    );

    // Painel de instrucao no canto inferior direito.
    Rectangle instructionPanel = {
        755, 695, 325, 48
    };

    DrawRectangleRounded(
        instructionPanel,
        0.25f,
        8,
        (Color){5, 15, 28, 225}
    );

    DrawRectangleRoundedLinesEx(
        instructionPanel,
        0.25f,
        8,
        2,
        (Color){40, 190, 230, 230}
    );

    const char *instruction =
        "ENTER / ESPACO  >  CONTINUAR";

    int textWidth = MeasureText(instruction, 16);

    DrawText(
        instruction,
        (int)(instructionPanel.x +
        (instructionPanel.width - textWidth) / 2),
        711,
        16,
        WHITE
    );
}

void Memory_Unload(void)
{
    for (int i = 0; i < TOTAL_SCENES; i++) {
        if (scenes[i].id != 0) {
            UnloadTexture(scenes[i]);
            scenes[i].id = 0;
        }
    }

    memoryActive = false;
}
