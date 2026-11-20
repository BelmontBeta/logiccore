
#include "room.h"

Rectangle Room_GetBounds(void)
{
    // Área horizontal onde BIT pode caminhar
    return (Rectangle){70, 570, 960, 50};
}

void Room_Draw(void)
{
    // Fundo da sala
    ClearBackground((Color){18, 22, 32, 255});

    // Parede do fundo
    DrawRectangle(50, 80, 1000, 490, (Color){30, 38, 52, 255});

    // Linha superior da parede
    DrawRectangle(50, 80, 1000, 8, DARKGRAY);

    // Chão
    DrawRectangle(50, 570, 1000, 50, (Color){65, 72, 85, 255});

    // Linha do chão
    DrawRectangle(50, 570, 1000, 4, LIGHTGRAY);

    // Porta de saída
    DrawRectangle(900, 350, 100, 220, (Color){20, 100, 120, 255});
    DrawRectangleLines(900, 350, 100, 220, SKYBLUE);

    // Identificação da porta
    DrawText("SAIDA", 922, 450, 20, WHITE);

    // Identificação da empresa
    DrawText("NEXORA INDUSTRIES", 75, 105, 20, LIGHTGRAY);
}