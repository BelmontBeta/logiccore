
#ifndef ROOM_H
#define ROOM_H

#include "raylib.h"

// Retorna os limites da área onde BIT pode andar
Rectangle Room_GetBounds(void);

// Desenha a sala
void Room_Draw(void);

#endif
