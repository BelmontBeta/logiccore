#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>

#define SCREEN_W 900
#define SCREEN_H 620
#define PHASE_COUNT 6
#define STEPS_PER_PHASE 5
#define RULE_COUNT 5
#define START_LIVES 5
#define ERRORS_FOR_HINT 3

typedef struct {
    const char *difficulty;
    const char *title;
    const char *expressions[STEPS_PER_PHASE + 1];
    int correctRules[STEPS_PER_PHASE];
    const char *hints[STEPS_PER_PHASE];
} Phase;

/*
    Indices das regras:
    0 = Contradicao
    1 = Elemento neutro
    2 = Distributiva
    3 = Terceiro excluido
    4 = De Morgan
*/
static const char *ruleNames[RULE_COUNT] = {
    "Contradicao",
    "Elemento neutro",
    "Distributiva",
    "Terceiro excluido",
    "De Morgan"
};

static const Phase phases[PHASE_COUNT] = {
    {
        "FACIL",
        "FASE 1 - FIREWALL DE ENTRADA",
        {
            "[(p ^ q) v (p ^ ~q)] v (r ^ ~r)",
            "[(p ^ q) v (p ^ ~q)] v F",
            "[(p ^ q) v (p ^ ~q)]",
            "p ^ (q v ~q)",
            "p ^ V",
            "p"
        },
        {0, 1, 2, 3, 1},
        {
            "Uma proposicao e sua negacao juntas resultam em F.",
            "Na disjuncao, X v F equivale a X.",
            "Procure o fator comum nas duas partes.",
            "Uma proposicao ou sua negacao resulta em V.",
            "Na conjuncao, X ^ V equivale a X."
        }
    },

    {
        "FACIL",
        "FASE 2 - TERMINAL DE SEGURANCA",
        {
            "(~(a ^ b) v (c ^ ~c)) ^ (d v ~d)",
            "((~a v ~b) v (c ^ ~c)) ^ (d v ~d)",
            "((~a v ~b) v F) ^ (d v ~d)",
            "(~a v ~b) ^ (d v ~d)",
            "(~a v ~b) ^ V",
            "~a v ~b"
        },
        {4, 0, 1, 3, 1},
        {
            "De Morgan transforma a negacao de uma conjuncao.",
            "Uma proposicao e sua negacao juntas resultam em F.",
            "Na disjuncao, X v F equivale a X.",
            "Uma proposicao ou sua negacao resulta em V.",
            "Na conjuncao, X ^ V equivale a X."
        }
    },

    {
        "MEDIA",
        "FASE 3 - NUCLEO DE DADOS",
        {
            "([(x ^ y) v (x ^ z)] v (t ^ ~t)) ^ (u v ~u)",
            "([x ^ (y v z)] v (t ^ ~t)) ^ (u v ~u)",
            "([x ^ (y v z)] v F) ^ (u v ~u)",
            "[x ^ (y v z)] ^ (u v ~u)",
            "[x ^ (y v z)] ^ V",
            "x ^ (y v z)"
        },
        {2, 0, 1, 3, 1},
        {
            "Coloque o fator comum x em evidencia.",
            "Uma proposicao e sua negacao juntas resultam em F.",
            "Na disjuncao, X v F equivale a X.",
            "Uma proposicao ou sua negacao resulta em V.",
            "Na conjuncao, X ^ V equivale a X."
        }
    },

    {
        "MEDIA",
        "FASE 4 - REDE CENTRAL",
        {
            "(~(m v n) v (s ^ ~s)) ^ (t v ~t)",
            "((~m ^ ~n) v (s ^ ~s)) ^ (t v ~t)",
            "((~m ^ ~n) v F) ^ (t v ~t)",
            "(~m ^ ~n) ^ (t v ~t)",
            "(~m ^ ~n) ^ V",
            "~m ^ ~n"
        },
        {4, 0, 1, 3, 1},
        {
            "De Morgan transforma a negacao de uma disjuncao.",
            "Uma proposicao e sua negacao juntas resultam em F.",
            "Na disjuncao, X v F equivale a X.",
            "Uma proposicao ou sua negacao resulta em V.",
            "Na conjuncao, X ^ V equivale a X."
        }
    },

    {
        "DIFICIL",
        "FASE 5 - COFRE FINAL",
        {
            "([(u ^ v) v (u ^ w)] v (x ^ ~x)) ^ (y v ~y)",
            "([u ^ (v v w)] v (x ^ ~x)) ^ (y v ~y)",
            "([u ^ (v v w)] v F) ^ (y v ~y)",
            "[u ^ (v v w)] ^ (y v ~y)",
            "[u ^ (v v w)] ^ V",
            "u ^ (v v w)"
        },
        {2, 0, 1, 3, 1},
        {
            "Coloque o fator comum u em evidencia.",
            "Uma proposicao e sua negacao juntas resultam em F.",
            "Na disjuncao, X v F equivale a X.",
            "Uma proposicao ou sua negacao resulta em V.",
            "Na conjuncao, X ^ V equivale a X."
        }
    },

    {
        "DIFICIL",
        "FASE 6 - NUCLEO PRINCIPAL",
        {
            "(~(j ^ k) v (l ^ ~l)) ^ (m v ~m)",
            "((~j v ~k) v (l ^ ~l)) ^ (m v ~m)",
            "((~j v ~k) v F) ^ (m v ~m)",
            "(~j v ~k) ^ (m v ~m)",
            "(~j v ~k) ^ V",
            "~j v ~k"
        },
        {4, 0, 1, 3, 1},
        {
            "De Morgan transforma a negacao de uma conjuncao.",
            "Uma proposicao e sua negacao juntas resultam em F.",
            "Na disjuncao, X v F equivale a X.",
            "Uma proposicao ou sua negacao resulta em V.",
            "Na conjuncao, X ^ V equivale a X."
        }
    }
};

static int phaseIndex = 0;
static int stepIndex = 0;
static int lives = START_LIVES;
static int errorsThisStep = 0;

static bool introVisible = true;
static bool gameOver = false;
static bool gameWon = false;

static char message[160] =
    "Selecione a regra equivalente para continuar.";

static bool Button(Rectangle bounds, const char *text)
{
    Vector2 mouse = GetMousePosition();
    bool hover = CheckCollisionPointRec(mouse, bounds);

    Color fill = hover
        ? (Color){57, 68, 89, 255}
        : (Color){36, 43, 59, 255};

    DrawRectangleRounded(bounds, 0.12f, 6, fill);
    DrawRectangleRoundedLinesEx(bounds, 0.12f, 6, 2.0f, SKYBLUE);

    int fontSize = 17;
    int textWidth = MeasureText(text, fontSize);

    DrawText(
        text,
        (int)(bounds.x + (bounds.width - textWidth) / 2),
        (int)(bounds.y + (bounds.height - fontSize) / 2),
        fontSize,
        RAYWHITE
    );

    return hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

static void ResetGame(void)
{
    phaseIndex = 0;
    stepIndex = 0;
    lives = START_LIVES;
    errorsThisStep = 0;

    introVisible = true;
    gameOver = false;
    gameWon = false;

    snprintf(
        message,
        sizeof(message),
        "Selecione a regra equivalente para continuar."
    );
}

static void ChooseRule(int selectedRule)
{
    if (gameOver || gameWon || introVisible)
        return;

    const Phase *phase = &phases[phaseIndex];

    if (selectedRule == phase->correctRules[stepIndex])
    {
        errorsThisStep = 0;
        stepIndex++;

        snprintf(message, sizeof(message), "Regra correta.");

        if (stepIndex >= STEPS_PER_PHASE)
        {
            phaseIndex++;
            stepIndex = 0;

            if (phaseIndex >= PHASE_COUNT)
            {
                gameWon = true;
            }
            else
            {
                lives = START_LIVES;
                errorsThisStep = 0;

                snprintf(
                    message,
                    sizeof(message),
                    "Fase superada! Suas vidas foram restauradas."
                );
            }
        }
    }
    else
    {
        lives--;
        errorsThisStep++;

        if (lives <= 0)
        {
            lives = 0;
            gameOver = true;
            snprintf(message, sizeof(message), "Voce ficou sem vidas.");
        }
        else if (errorsThisStep >= ERRORS_FOR_HINT)
        {
            snprintf(
                message,
                sizeof(message),
                "Dica: %s",
                phase->hints[stepIndex]
            );
        }
        else
        {
            snprintf(
                message,
                sizeof(message),
                "Resposta incorreta. Voce perdeu uma vida."
            );
        }
    }
}

static void DrawIntro(void)
{
    DrawRectangle(
        0, 0, SCREEN_W, SCREEN_H,
        (Color){2, 5, 12, 220}
    );

    Rectangle panel = {150, 145, 600, 330};

    DrawRectangleRounded(
        panel,
        0.08f,
        8,
        (Color){26, 33, 47, 255}
    );

    DrawRectangleRoundedLinesEx(
        panel,
        0.08f,
        8,
        2,
        SKYBLUE
    );

    const char *title = "MISSAO: DESATIVAR O FIREWALL";

    DrawText(
        title,
        SCREEN_W / 2 - MeasureText(title, 25) / 2,
        180,
        25,
        SKYBLUE
    );

    DrawText(
        "Resolva as questoes logicas para avancar por 6 fases.",
        195, 240, 17, RAYWHITE
    );
    DrawText(
        "As fases estao divididas em 2 faceis, 2 medias e 2 dificeis.",
        195, 275, 17, LIGHTGRAY
    );
    DrawText(
        "Voce tem 5 vidas por fase; elas sao restauradas ao avancar.",
        195, 310, 17, LIGHTGRAY
    );
    DrawText(
        "Apos 3 erros na mesma etapa, aparece uma dica.",
        195, 345, 17, GOLD
    );

    Rectangle startButton = {325, 395, 250, 55};

    if (Button(startButton, "INICIAR"))
        introVisible = false;
}

static void DrawEndOverlay(
    const char *title,
    const char *subtitle,
    Color accent
)
{
    DrawRectangle(
        0, 0, SCREEN_W, SCREEN_H,
        (Color){2, 5, 12, 220}
    );

    Rectangle panel = {150, 190, 600, 250};

    DrawRectangleRounded(
        panel,
        0.08f,
        8,
        (Color){26, 33, 47, 255}
    );

    DrawRectangleRoundedLinesEx(
        panel,
        0.08f,
        8,
        2,
        accent
    );

    DrawText(
        title,
        SCREEN_W / 2 - MeasureText(title, 30) / 2,
        230,
        30,
        accent
    );

    DrawText(
        subtitle,
        SCREEN_W / 2 - MeasureText(subtitle, 18) / 2,
        290,
        18,
        RAYWHITE
    );

    Rectangle restartButton = {325, 345, 250, 55};

    if (Button(restartButton, "JOGAR NOVAMENTE"))
        ResetGame();
}

int main(void)
{
    InitWindow(
        SCREEN_W,
        SCREEN_H,
        "Logic Core - Terminal de Simplificacao"
    );

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground((Color){17, 21, 32, 255});

        DrawText(
            "TERMINAL DE SIMPLIFICACAO",
            40, 28, 30, SKYBLUE
        );

        DrawText(
            "Escolha a regra equivalente para simplificar a expressao.",
            42, 70, 17, LIGHTGRAY
        );

        if (!gameWon && !gameOver)
        {
            const Phase *phase = &phases[phaseIndex];

            DrawText(phase->title, 42, 112, 18, GOLD);

            DrawText(
                TextFormat("FASE %d / %d", phaseIndex + 1, PHASE_COUNT),
                700, 112, 16, LIGHTGRAY
            );

            DrawText(phase->difficulty, 700, 138, 16, GOLD);

            DrawText(
                TextFormat("VIDAS: %d", lives),
                700, 165, 20,
                lives <= 2 ? RED : GREEN
            );

            DrawText("EXPRESSAO ATUAL", 45, 165, 17, GOLD);

            DrawRectangleRounded(
                (Rectangle){45, 198, 810, 100},
                0.1f,
                8,
                (Color){26, 33, 47, 255}
            );

            DrawText(
                phase->expressions[stepIndex],
                65, 234, 22, WHITE
            );

            const Rectangle buttons[RULE_COUNT] = {
                {45, 365, 250, 52},
                {315, 365, 250, 52},
                {585, 365, 250, 52},
                {45, 431, 250, 52},
                {315, 431, 250, 52}
            };

            /*
                Os botoes ficam sempre nesta ordem.
                O que muda e a regra correta de cada questao/fase.
            */
            for (int i = 0; i < RULE_COUNT; i++)
            {
                if (Button(buttons[i], ruleNames[i]))
                    ChooseRule(i);
            }

            DrawText(message, 45, 515, 17, LIGHTGRAY);
            DrawText(
                "Cada resposta incorreta remove uma vida.",
                45, 555, 14, GRAY
            );
        }

        if (introVisible)
        {
            DrawIntro();
        }
        else if (gameOver)
        {
            DrawEndOverlay(
                "FIM DE JOGO",
                "Voce ficou sem vidas. O firewall bloqueou o acesso.",
                RED
            );
        }
        else if (gameWon)
        {
            DrawEndOverlay(
                "FIREWALL DESATIVADO!",
                "Voce completou as seis fases.",
                GREEN
            );
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}