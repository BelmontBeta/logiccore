#include "room.h"

#define TOTAL_ROOMS 3

static Texture2D backgrounds[TOTAL_ROOMS];
static bool backgroundsLoaded = false;

void Room_Init(void)
{
    backgrounds[0] = LoadTexture("assets/backgrounds/room1.png");
    backgrounds[1] = LoadTexture("assets/backgrounds/room2.png");
    backgrounds[2] = LoadTexture("assets/backgrounds/room3.png");

    backgroundsLoaded = true;
}

void Room_Unload(void)
{
    if (!backgroundsLoaded)
        return;

    for (int i = 0; i < TOTAL_ROOMS; i++)
    {
        if (backgrounds[i].id != 0)
            UnloadTexture(backgrounds[i]);
    }

    backgroundsLoaded = false;
}

Rectangle Room_GetBounds(void)
{
    return (Rectangle){70, 570, 960, 50};
}

void Room_Draw(int roomNumber)
{
    ClearBackground((Color){18, 22, 32, 255});

    if (!backgroundsLoaded ||
        roomNumber < 1 ||
        roomNumber > TOTAL_ROOMS)
        return;

    Texture2D background = backgrounds[roomNumber - 1];

    if (background.id == 0)
        return;

    DrawTexturePro(
        background,
        (Rectangle){
            0, 0,
            (float)background.width,
            (float)background.height
        },
        (Rectangle){0, 0, 1100, 760},
        (Vector2){0, 0},
        0.0f,
        WHITE
    );
}