#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>

#define SCREEN_W 900
#define SCREEN_H 620
#define PHASE_COUNT 5
#define STEPS_PER_PHASE 5
#define START_TIME 120.0f

typedef struct {
    const char *title;
    const char *expressions[STEPS_PER_PHASE + 1];
} Phase;

static const Phase phases[PHASE_COUNT] = {
    {
        "FASE 1 - FIREWALL DE ENTRADA",
        {
            "[(p ^ q) v (p ^ ~q)] v (r ^ ~r)",
            "[(p ^ q) v (p ^ ~q)] v F",
            "[(p ^ q) v (p ^ ~q)]",
            "p ^ (q v ~q)",
            "p ^ V",
            "p"
        }
    },
    {
        "FASE 2 - TERMINAL DE SEGURANCA",
        {
            "[(a ^ b) v (a ^ ~b)] v (c ^ ~c)",
            "[(a ^ b) v (a ^ ~b)] v F",
            "[(a ^ b) v (a ^ ~b)]",
            "a ^ (b v ~b)",
            "a ^ V",
            "a"
        }
    },
    {
        "FASE 3 - NUCLEO DE DADOS",
        {
            "[(x ^ y) v (x ^ ~y)] v (z ^ ~z)",
            "[(x ^ y) v (x ^ ~y)] v F",
            "[(x ^ y) v (x ^ ~y)]",
            "x ^ (y v ~y)",
            "x ^ V",
            "x"
        }
    },
    {
        "FASE 4 - REDE CENTRAL",
        {
            "[(m ^ n) v (m ^ ~n)] v (s ^ ~s)",
            "[(m ^ n) v (m ^ ~n)] v F",
            "[(m ^ n) v (m ^ ~n)]",
            "m ^ (n v ~n)",
            "m ^ V",
            "m"
        }
    },
    {
        "FASE 5 - COFRE FINAL",
        {
            "[(u ^ v) v (u ^ ~v)] v (w ^ ~w)",
            "[(u ^ v) v (u ^ ~v)] v F",
            "[(u ^ v) v (u ^ ~v)]",
            "u ^ (v v ~v)",
            "u ^ V",
            "u"
        }
    }
};

/* Ordem correta dos conectivos para os cinco passos de cada fase. */
static const int correctRules[STEPS_PER_PHASE] = {
    0,  /* Contradicao */
    1,  /* Elemento neutro */
    2,  /* Distributiva */
    3,  /* Terceiro excluido */
    1   /* Elemento neutro */
};

static const char *ruleNames[] = {
    "Contradicao",
    "Elemento neutro",
    "Distributiva",
    "Terceiro excluido",
    "De Morgan"
};

static const char *stepLabels[STEPS_PER_PHASE] = {
    "r ^ ~r = F",
    "X v F = X",
    "(p^q) v (p^~q) = p^(q v ~q)",
    "q v ~q = V",
    "p ^ V = p"
};

static int phaseIndex = 0;
static int stepIndex = 0;
static float timeLeft = START_TIME;
static bool introVisible = true;
static bool gameOver = false;
static bool gameWon = false;

static char message[160] =
    "Selecione a regra equivalente para continuar.";

static bool Button(Rectangle bounds, const char *text, bool enabled)
{
    Vector2 mouse = GetMousePosition();
    bool hover = CheckCollisionPointRec(mouse, bounds);

    Color fill = !enabled ? (Color){40, 42, 49, 255} :
                 hover ? (Color){57, 68, 89, 255} :
                         (Color){36, 43, 59, 255};

    DrawRectangleRounded(bounds, 0.12f, 6, fill);
    DrawRectangleRoundedLinesEx(
        bounds, 0.12f, 6, 2.0f,
        enabled ? SKYBLUE : GRAY
    );

    int fontSize = 17;
    int textWidth = MeasureText(text, fontSize);

    DrawText(text,
             (int)(bounds.x + (bounds.width - textWidth) / 2),
             (int)(bounds.y + (bounds.height - fontSize) / 2),
             fontSize,
             enabled ? RAYWHITE : GRAY);

    return enabled && hover &&
           IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

static void ResetGame(void)
{
    phaseIndex = 0;
    stepIndex = 0;
    timeLeft = START_TIME;
    introVisible = true;
    gameOver = false;
    gameWon = false;

    snprintf(message, sizeof(message),
             "Selecione a regra equivalente para continuar.");
}

static void ChooseRule(int rule)
{
    if (gameOver || gameWon || introVisible) return;

    if (rule == correctRules[stepIndex]) {
        stepIndex++;
        snprintf(message, sizeof(message), "Regra correta.");

        if (stepIndex >= STEPS_PER_PHASE) {
            phaseIndex++;
            stepIndex = 0;

            if (phaseIndex >= PHASE_COUNT) {
                gameWon = true;
            } else {
                snprintf(message, sizeof(message),
                         "Fase superada! A proxima trava foi localizada.");
            }
        }
    } else {
        timeLeft -= 2.0f;
        if (timeLeft < 0.0f) timeLeft = 0.0f;

        snprintf(message, sizeof(message),
                 "Regra incorreta! Voce perdeu 2 segundos.");

        if (timeLeft <= 0.0f) {
            gameOver = true;
        }
    }
}

static void DrawIntro(void)
{
    DrawRectangle(0, 0, SCREEN_W, SCREEN_H,
                  (Color){2, 5, 12, 220});

    Rectangle panel = {150, 145, 600, 330};
    DrawRectangleRounded(panel, 0.08f, 8,
                         (Color){26, 33, 47, 255});
    DrawRectangleRoundedLinesEx(panel, 0.08f, 8, 2, SKYBLUE);

    const char *title = "MISSAO: DESATIVAR O FIREWALL";
    DrawText(title,
             SCREEN_W / 2 - MeasureText(title, 25) / 2,
             180, 25, SKYBLUE);

    DrawText("Resolva as equivalencias logicas para avancar por 5 fases.",
             195, 240, 17, RAYWHITE);
    DrawText("Escolha a regra que transforma a expressao atual.",
             195, 275, 17, LIGHTGRAY);
    DrawText("Cada erro desconta 2 segundos do cronometro.",
             195, 310, 17, LIGHTGRAY);
    DrawText("O tempo comeca quando voce pressionar INICIAR.",
             195, 345, 17, GOLD);

    Rectangle startButton = {325, 395, 250, 55};

    if (Button(startButton, "INICIAR", true)) {
        introVisible = false;
    }
}

static void DrawEndOverlay(const char *title,
                           const char *subtitle,
                           Color accent)
{
    DrawRectangle(0, 0, SCREEN_W, SCREEN_H,
                  (Color){2, 5, 12, 220});

    Rectangle panel = {170, 190, 560, 250};
    DrawRectangleRounded(panel, 0.08f, 8,
                         (Color){26, 33, 47, 255});
    DrawRectangleRoundedLinesEx(panel, 0.08f, 8, 2, accent);

    DrawText(title,
             SCREEN_W / 2 - MeasureText(title, 30) / 2,
             230, 30, accent);

    DrawText(subtitle,
             SCREEN_W / 2 - MeasureText(subtitle, 18) / 2,
             290, 18, RAYWHITE);

    Rectangle restartButton = {325, 345, 250, 55};

    if (Button(restartButton, "JOGAR NOVAMENTE", true)) {
        ResetGame();
    }
}

int main(void)
{
    InitWindow(SCREEN_W, SCREEN_H, "Logic Core - Terminal de Simplificacao");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        float delta = GetFrameTime();

        if (!introVisible && !gameOver && !gameWon) {
            timeLeft -= delta;

            if (timeLeft <= 0.0f) {
                timeLeft = 0.0f;
                gameOver = true;
            }
        }

        BeginDrawing();
        ClearBackground((Color){17, 21, 32, 255});

        DrawText("TERMINAL DE SIMPLIFICACAO",
                 40, 28, 30, SKYBLUE);

        DrawText("Escolha equivalencias logicas para desativar o firewall.",
                 42, 70, 17, LIGHTGRAY);

        if (!gameWon && !gameOver) {
            DrawText(phases[phaseIndex].title, 42, 112, 18, GOLD);

            DrawText(TextFormat("FASE %d / %d", phaseIndex + 1, PHASE_COUNT),
                     700, 115, 16, LIGHTGRAY);

            Color timerColor = timeLeft <= 20.0f ? RED : RAYWHITE;
            DrawText(TextFormat("TEMPO: %02d", (int)timeLeft),
                     700, 145, 22, timerColor);

            DrawText("EXPRESSAO ATUAL", 45, 165, 17, GOLD);

            DrawRectangleRounded(
                (Rectangle){45, 198, 810, 100},
                0.1f, 8, (Color){26, 33, 47, 255}
            );

            DrawText(phases[phaseIndex].expressions[stepIndex],
                     65, 234, 22, WHITE);

            DrawText("Transformacao esperada:",
                     45, 320, 17, LIGHTGRAY);

            DrawText(stepLabels[stepIndex],
                     45, 348, 18, SKYBLUE);

            const Rectangle buttons[] = {
                {45, 400, 250, 52},
                {315, 400, 250, 52},
                {585, 400, 250, 52},
                {45, 466, 250, 52},
                {315, 466, 250, 52}
            };

            for (int i = 0; i < 5; i++) {
                if (Button(buttons[i], ruleNames[i], true)) {
                    ChooseRule(i);
                }
            }

            DrawText(message, 45, 550, 17,
                     timeLeft <= 20.0f ? RED : LIGHTGRAY);

            DrawText("Uma resposta errada remove 2 segundos.",
                     45, 580, 14, GRAY);
        }

        if (introVisible) {
            DrawIntro();
        } else if (gameOver) {
            DrawEndOverlay("TEMPO ESGOTADO",
                           "O firewall bloqueou o acesso.",
                           RED);
        } else if (gameWon) {
            DrawEndOverlay("FIREWALL DESATIVADO!",
                           "Voce completou as cinco fases.",
                           GREEN);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
