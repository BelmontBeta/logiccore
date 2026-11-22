
#ifndef PLAYER_H
#define PLAYER_H

#include "../core/game_state.h"

void Player_Init(Player *player, Vector2 startPosition);
void Player_Update(Player *player, float dt, Rectangle roomBounds);
void Player_Draw(const Player *player);
void Player_Unload(Player *player);

#endif
