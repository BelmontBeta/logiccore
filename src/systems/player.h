
#ifndef PLAYER_H
#define PLAYER_H

#include "../core/game_state.h"

// Inicializa o jogador
void Player_Init(Player *player, Vector2 startPosition);

// Atualiza a movimentação
void Player_Update(Player *player, float dt, Rectangle roomBounds);

// Desenha o jogador
void Player_Draw(const Player *player);

#endif
