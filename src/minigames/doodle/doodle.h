
#ifndef DOODLE_H
#define DOODLE_H

#include "raylib.h"

typedef enum {
    DOODLE_RUNNING,
    DOODLE_COMPLETED,
    DOODLE_GAME_OVER
} DoodleResult;

void Doodle_Init(int *campaignLives);
DoodleResult Doodle_Update(float dt);
void Doodle_Draw(void);
void Doodle_Unload(void);

#endif
