
#ifndef TERMINAL_H
#define TERMINAL_H

#include "raylib.h"

typedef enum {
    TERMINAL_RUNNING,
    TERMINAL_COMPLETED,
    TERMINAL_GAME_OVER
} TerminalResult;

void Terminal_Init(int *campaignLives);
TerminalResult Terminal_Update(float dt);
void Terminal_Draw(void);
void Terminal_Unload(void);

#endif
