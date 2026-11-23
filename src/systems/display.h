#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdbool.h>

// Resolucao logica do jogo: todo o desenho e a logica usam este tamanho.
// A janela pode ter qualquer tamanho; a imagem e escalada com barras pretas.
#define GAME_WIDTH  1100
#define GAME_HEIGHT 760

// Chamar depois de InitWindow().
void Display_Init(void);
void Display_Unload(void);

// Chamar uma vez por frame, antes de Game_Update().
// Trata a tecla F11 e ajusta o mapeamento do mouse.
void Display_Update(void);

// Envolvem o desenho do jogo (substituem BeginDrawing/EndDrawing).
void Display_BeginRender(void);
void Display_EndRender(void);

// Resolucoes disponiveis (somente as que cabem no monitor).
int  Display_GetResolutionCount(void);
int  Display_GetResolutionIndex(void);
void Display_GetResolution(int index, int *width, int *height);
void Display_SetResolution(int index);

bool Display_IsFullscreen(void);
void Display_ToggleFullscreen(void);

#endif
