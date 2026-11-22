
#include "terminal.h"

#include <stdbool.h>
#include <stdio.h>

#define PHASE_COUNT 5
#define STEPS_PER_PHASE 5
#define RULE_COUNT 5
#define ERRORS_FOR_HINT 3

typedef struct {
    const char *difficulty;
    const char *title;
    const char *expressions[STEPS_PER_PHASE + 1];
    int correctRules[STEPS_PER_PHASE];
    const char *hints[STEPS_PER_PHASE];
} Phase;

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
    }
};

static int phaseIndex;
static int stepIndex;
static int errorsThisStep;
static int *campaignLives;

static bool introVisible;
static bool gameWon;
static bool gameOver;

static char message[180];

static Rectangle ruleButtons[RULE_COUNT] = {
    {100, 475, 270, 55},
    {400, 475, 270, 55},
    {700, 475, 270, 55},
    {250, 550, 270, 55},
    {550, 550, 270, 55}
};

static void SetMessage(const char *text)
{
    snprintf(message, sizeof(message), "%s", text);
}

static bool Hover(Rectangle bounds)
{
    return CheckCollisionPointRec(GetMousePosition(), bounds);
}

static bool Clicked(Rectangle bounds)
{
    return Hover(bounds) &&
           IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

static void DrawButton(Rectangle bounds, const char *text)
{
    bool hover = Hover(bounds);

    Color fill = hover
        ? (Color){57, 68, 89, 255}
        : (Color){36, 43, 59, 255};

    DrawRectangleRounded(bounds, 0.12f, 6, fill);
    DrawRectangleRoundedLinesEx(bounds, 0.12f, 6, 2, SKYBLUE);

    int fontSize = 17;
    int textWidth = MeasureText(text, fontSize);

    DrawText(
        text,
        (int)(bounds.x + (bounds.width - textWidth) / 2),
        (int)(bounds.y + (bounds.height - fontSize) / 2),
        fontSize,
        RAYWHITE
    );
}

static void ResetTerminal(void)
{
    phaseIndex = 0;
    stepIndex = 0;
    errorsThisStep = 0;

    introVisible = true;
    gameOver = false;
    gameWon = false;

    SetMessage("Selecione a regra equivalente para continuar.");
}

static void ChooseRule(int selectedRule)
{
    if (gameOver || gameWon || introVisible)
        return;

    if (campaignLives == NULL || *campaignLives <= 0) {
        gameOver = true;
        return;
    }

    const Phase *phase = &phases[phaseIndex];

    if (selectedRule == phase->correctRules[stepIndex]) {
        errorsThisStep = 0;
        stepIndex++;

        SetMessage("Regra correta! Continue.");

        if (stepIndex >= STEPS_PER_PHASE) {
            phaseIndex++;
            stepIndex = 0;

            if (phaseIndex >= PHASE_COUNT) {
                gameWon = true;
                SetMessage("Todas as fases foram concluidas!");
            }
            else {
                SetMessage("Fase superada! Proxima etapa liberada.");
            }
        }
    }
    else {
        (*campaignLives) -= 20;

        if (*campaignLives < 0)
            *campaignLives = 0;

        errorsThisStep++;

        if (*campaignLives <= 0) {
            gameOver = true;
            SetMessage("Bateria do BIT esgotada.");
        }

        else if (errorsThisStep >= ERRORS_FOR_HINT) {
            snprintf(
                message,
                sizeof(message),
                "Dica: %s",
                phase->hints[stepIndex]
            );
        }
        else {
            SetMessage("Resposta incorreta. Bateria reduzida em 20%.");
        }
    }
}

void Terminal_Init(int *lives)
{
    campaignLives = lives;
    ResetTerminal();
}

TerminalResult Terminal_Update(float dt)
{
    (void)dt;

    if (campaignLives == NULL || *campaignLives <= 0) {
        gameOver = true;
        return TERMINAL_GAME_OVER;
    }

    if (gameOver)
        return TERMINAL_GAME_OVER;

    if (gameWon)
        return TERMINAL_COMPLETED;

    if (introVisible) {
        if (Clicked((Rectangle){400, 490, 300, 55})) {
            introVisible = false;
            SetMessage("Selecione a regra equivalente.");
        }

        return TERMINAL_RUNNING;
    }

    for (int i = 0; i < RULE_COUNT; i++) {
        if (Clicked(ruleButtons[i])) {
            ChooseRule(i);
            break;
        }
    }

    if (gameOver)
        return TERMINAL_GAME_OVER;

    if (gameWon)
        return TERMINAL_COMPLETED;

    return TERMINAL_RUNNING;
}

void Terminal_Draw(void)
{
    DrawRectangle(70, 105, 960, 605, (Color){17, 21, 32, 255});

    DrawText(
        "TERMINAL DE SIMPLIFICACAO",
        100, 125, 30, SKYBLUE
    );

    DrawText(
        TextFormat("VIDAS: %d", campaignLives ? *campaignLives : 0),
        820, 130, 20, GREEN
    );

    if (introVisible) {
        DrawText(
            "NEXORA INDUSTRIES - ACESSO AO NUCLEO",
            220, 280, 23, GOLD
        );

        DrawText(
            "Simplifique as expressoes usando as regras logicas.",
            190, 345, 19, WHITE
        );

        DrawText(
            "Voce precisa concluir cinco fases com cinco passos cada.",
            175, 385, 17, LIGHTGRAY
        );

        DrawText(
            "Cada erro consome uma vida compartilhada da campanha.",
            175, 420, 17, LIGHTGRAY
        );

        DrawButton(
            (Rectangle){400, 490, 300, 55},
            "INICIAR"
        );

        return;
    }

    if (gameOver) {
        DrawText("FIM DE JOGO", 390, 330, 38, RED);
        DrawText(
            "As vidas da campanha acabaram.",
            330, 390, 20, WHITE
        );
        return;
    }

    if (gameWon) {
        DrawText("TERMINAL DESBLOQUEADO!", 250, 315, 34, GREEN);
        DrawText(
            "Todas as cinco fases foram concluidas.",
            300, 380, 20, WHITE
        );
        return;
    }

    const Phase *phase = &phases[phaseIndex];

    DrawText(
        TextFormat("FASE %d / %d", phaseIndex + 1, PHASE_COUNT),
        820, 165, 18, LIGHTGRAY
    );

    DrawText(phase->title, 100, 200, 21, GOLD);
    DrawText(phase->difficulty, 100, 235, 18, GOLD);

    DrawText(
        TextFormat("PASSO %d / %d", stepIndex + 1, STEPS_PER_PHASE),
        820, 235, 18, SKYBLUE
    );

    DrawText("EXPRESSAO ATUAL:", 100, 290, 18, GOLD);

    DrawRectangleRounded(
        (Rectangle){100, 325, 850, 90},
        0.08f,
        6,
        (Color){35, 43, 59, 255}
    );

    DrawText(
        phase->expressions[stepIndex],
        120, 358, 20, WHITE
    );

    DrawText(
        "ESCOLHA A REGRA EQUIVALENTE:",
        100, 440, 18, GOLD
    );

    for (int i = 0; i < RULE_COUNT; i++) {
        DrawButton(ruleButtons[i], ruleNames[i]);
    }

    DrawText(message, 100, 635, 18, LIGHTGRAY);

    DrawText(
        "Apos 3 erros no mesmo passo, uma dica sera exibida.",
        100, 670, 15, GRAY
    );
}

void Terminal_Unload(void)
{
    campaignLives = NULL;
}
