
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

    // Sprites do BIT
    Texture2D idleFrames[2];
    Texture2D walkFrames[3];

    // Controle da animação
    int currentFrame;
    float animationTimer;
    bool moving;
    bool facingLeft;
} Player;

// Estado geral da campanha
typedef struct {
    Player player;

    int lives;
    int currentRoom;

    bool gameRunning;
} GameState;

#endif
