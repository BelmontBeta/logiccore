
#ifndef MEMORY_H
#define MEMORY_H

#include <stdbool.h>

typedef enum {
    MEMORY_INTRO,
    MEMORY_BETWEEN_ROOMS,
    MEMORY_ROOM2,
    MEMORY_DISCOVERY,
    MEMORY_ENDING
} MemorySequence;

bool Memory_Init(void);
void Memory_Start(MemorySequence sequence);
bool Memory_Update(float dt);
void Memory_Draw(void);
void Memory_Unload(void);

#endif
