#ifndef ROOM_H
#define ROOM_H

#include "raylib.h"

void Room_Init(void);
void Room_Unload(void);
Rectangle Room_GetBounds(void);
void Room_Draw(int roomNumber);

#endif