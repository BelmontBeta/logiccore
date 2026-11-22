
#ifndef CIPHER_H
#define CIPHER_H

#include "raylib.h"

typedef enum {
    CIPHER_RUNNING,
    CIPHER_COMPLETED,
    CIPHER_GAME_OVER
} CipherResult;

void Cipher_Init(int *campaignLives);
CipherResult Cipher_Update(void);
void Cipher_Draw(void);
void Cipher_Unload(void);

#endif
