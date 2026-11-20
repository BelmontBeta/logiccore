
#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "raylib.h"
#include <stdbool.h>

// Informações do jogador
typedef struct {
    Vector2 position;
    Vector2 size;
    float speed;
    Color color;
} Player;

// Estado geral da campanha
typedef struct {
    Player player;

    int lives;
    int currentRoom;

    bool gameRunning;
} GameState;

#endif
