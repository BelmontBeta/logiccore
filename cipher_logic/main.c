#include "raylib.h"
#include <stdbool.h>
#include <string.h>

#define MAX_INPUT 128

static bool Button(Rectangle bounds, const char *text)
{
    Vector2 mouse = GetMousePosition();
    bool hover = CheckCollisionPointRec(mouse, bounds);

    DrawRectangleRec(bounds, hover ? (Color){ 57, 68, 89, 255 } :
                                     (Color){ 36, 43, 59, 255 });
    DrawRectangleLinesEx(bounds, 2, SKYBLUE);
    DrawText(text, (int)bounds.x + 15, (int)bounds.y + 14, 20, RAYWHITE);

    return hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

static void AddText(char *input, const char *text)
{
    if (strlen(input) + strlen(text) + 1 < MAX_INPUT)
        strcat(input, text);
}

int main(void)
{
    InitWindow(900, 620, "Logic Core - Cipher Logic");
    SetTargetFPS(60);

    char input[MAX_INPUT] = "";
    const char *message = "Traduza a frase usando os simbolos.";
    bool solved = false;

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground((Color){ 17, 21, 32, 255 });

        DrawText("CIPHER LOGIC", 40, 30, 34, SKYBLUE);
        DrawText("INTERCEPTACAO DE RADIO", 42, 80, 18, GOLD);

        DrawRectangleRounded((Rectangle){ 40, 120, 820, 115 }, 0.1f, 8,
                             (Color){ 26, 33, 47, 255 });
        DrawText("\"Se o sensor detectar movimento ou a luz apagar,", 60, 145, 20, WHITE);
        DrawText("o alarme principal aciona.\"", 60, 177, 20, WHITE);

        DrawText("p = sensor detecta movimento", 45, 255, 18, LIGHTGRAY);
        DrawText("q = luz apaga", 45, 282, 18, LIGHTGRAY);
        DrawText("r = alarme principal aciona", 45, 309, 18, LIGHTGRAY);

        DrawText("MONTE A FORMULA", 45, 350, 18, GOLD);
        DrawRectangleRounded((Rectangle){ 45, 382, 810, 62 }, 0.12f, 8,
                             (Color){ 26, 33, 47, 255 });
        DrawText(input[0] ? input : "_", 65, 400, 24, WHITE);

        if (Button((Rectangle){ 45, 470, 110, 50 }, "(")) AddText(input, "(");
        if (Button((Rectangle){ 165, 470, 110, 50 }, "p")) AddText(input, "p");
        if (Button((Rectangle){ 285, 470, 110, 50 }, "v")) AddText(input, " v ");
        if (Button((Rectangle){ 405, 470, 110, 50 }, "q")) AddText(input, "q");
        if (Button((Rectangle){ 525, 470, 110, 50 }, ")")) AddText(input, ")");
        if (Button((Rectangle){ 645, 470, 110, 50 }, "->")) AddText(input, " -> ");
        if (Button((Rectangle){ 765, 470, 90, 50 }, "r")) AddText(input, "r");

        if (Button((Rectangle){ 45, 535, 150, 48 }, "Apagar"))
        {
            size_t length = strlen(input);
            if (length > 0)
            {
                input[length - 1] = '\0';
                while (length > 1 && input[length - 2] == ' ')
                    input[--length - 1] = '\0';
            }
            solved = false;
        }

        if (Button((Rectangle){ 215, 535, 190, 48 }, "Verificar"))
        {
            solved = strcmp(input, "(p v q) -> r") == 0;
            message = solved ? "Correto! Os alarmes foram desativados." :
                               "Formula incorreta. Confira a ordem dos simbolos.";
        }

        if (Button((Rectangle){ 425, 535, 150, 48 }, "Limpar"))
        {
            input[0] = '\0';
            solved = false;
            message = "Traduza a frase usando os simbolos.";
        }

        DrawText(message, 45, 595, 18, solved ? GREEN : LIGHTGRAY);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}