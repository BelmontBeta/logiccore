
#ifndef GAME_H
#define GAME_H

#include "game_state.h"

// Inicialização e encerramento
void Game_Init(void);
void Game_Unload(void);

// Atualização e desenho
void Game_Update(float dt);
void Game_Draw(void);

#endif