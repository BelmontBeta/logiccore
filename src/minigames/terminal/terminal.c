
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

/* Paleta visual */

static const Color TERMINAL_BG = {12, 16, 27, 255};
static const Color TERMINAL_PANEL = {23, 31, 48, 255};
static const Color TERMINAL_CARD = {35, 46, 66, 255};
static const Color TERMINAL_HOVER = {52, 68, 92, 255};
static const Color TERMINAL_BORDER = {75, 110, 145, 255};
static const Color TERMINAL_TEXT = {240, 245, 252, 255};
static const Color TERMINAL_MUTED = {165, 180, 200, 255};
static const Color TERMINAL_CYAN = {80, 205, 235, 255};
static const Color TERMINAL_GREEN = {85, 220, 145, 255};
static const Color TERMINAL_RED = {245, 90, 100, 255};
static const Color TERMINAL_GOLD = {255, 200, 90, 255};

/* Estado do minigame */

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

/* Funcoes auxiliares */

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

static void DrawPanel(Rectangle bounds, Color color)
{
    DrawRectangleRounded(
        (Rectangle){
            bounds.x + 3,
            bounds.y + 4,
            bounds.width,
            bounds.height
        },
        0.06f,
        6,
        (Color){0, 0, 0, 65}
    );

    DrawRectangleRounded(
        bounds,
        0.06f,
        6,
        color
    );

    DrawRectangleRoundedLinesEx(
        bounds,
        0.06f,
        6,
        1.5f,
        TERMINAL_BORDER
    );
}

static void DrawCentered(
    const char *text,
    int centerX,
    int y,
    int fontSize,
    Color color
)
{
    int width = MeasureText(text, fontSize);

    DrawText(
        text,
        centerX - width / 2,
        y,
        fontSize,
        color
    );
}

static void DrawFitText(
    const char *text,
    int x,
    int y,
    int maxWidth,
    int fontSize,
    Color color
)
{
    while (
        MeasureText(text, fontSize) > maxWidth &&
        fontSize > 9
    ) {
        fontSize--;
    }

    DrawText(
        text,
        x,
        y,
        fontSize,
        color
    );
}

/* Elementos visuais */

static void DrawHeader(void)
{
    DrawRectangleRounded(
        (Rectangle){70, 105, 960, 55},
        0.08f,
        8,
        (Color){11, 22, 38, 255}
    );

    DrawRectangle(
        70, 157, 960, 3,
        TERMINAL_CYAN
    );

    DrawText(
        "TERMINAL LOGIC",
        95, 117, 27,
        TERMINAL_CYAN
    );

    DrawText(
        "SISTEMA DE SIMPLIFICACAO LOGICA",
        97, 143, 10,
        TERMINAL_MUTED
    );

    DrawRectangleRounded(
        (Rectangle){780, 115, 230, 34},
        0.3f,
        8,
        (Color){30, 45, 65, 255}
    );

    DrawCentered(
        TextFormat(
            "BATERIA: %d%%",
            campaignLives ? *campaignLives : 0
        ),
        895,
        125,
        15,
        TERMINAL_GREEN
    );
}

static void DrawProgress(void)
{
    int completed = phaseIndex * STEPS_PER_PHASE + stepIndex;
    int total = PHASE_COUNT * STEPS_PER_PHASE;

    float progress = (float)completed / total;

    if (progress < 0)
        progress = 0;

    if (progress > 1)
        progress = 1;

    DrawRectangleRounded(
        (Rectangle){100, 175, 850, 12},
        0.5f,
        8,
        (Color){35, 45, 60, 255}
    );

    if (progress > 0) {
        DrawRectangleRounded(
            (Rectangle){
                100, 175,
                850 * progress,
                12
            },
            0.5f,
            8,
            TERMINAL_CYAN
        );
    }

    DrawText(
        TextFormat(
            "%d%% CONCLUIDO",
            (int)(progress * 100)
        ),
        100, 194, 12,
        TERMINAL_MUTED
    );

    DrawText(
        TextFormat(
            "FASE %d / %d",
            phaseIndex + 1,
            PHASE_COUNT
        ),
        820, 194, 13,
        TERMINAL_CYAN
    );
}

static void DrawButton(
    Rectangle bounds,
    const char *text
)
{
    bool hover = Hover(bounds);

    Color fill = hover
        ? TERMINAL_HOVER
        : TERMINAL_CARD;

    DrawRectangleRounded(
        (Rectangle){
            bounds.x + 2,
            bounds.y + 3,
            bounds.width,
            bounds.height
        },
        0.12f,
        6,
        (Color){0, 0, 0, 55}
    );

    DrawRectangleRounded(
        bounds,
        0.12f,
        6,
        fill
    );

    DrawRectangleRoundedLinesEx(
        bounds,
        0.12f,
        6,
        hover ? 2 : 1,
        TERMINAL_CYAN
    );

    int fontSize = 17;

    while (
        MeasureText(text, fontSize) > bounds.width - 20 &&
        fontSize > 10
    ) {
        fontSize--;
    }

    DrawCentered(
        text,
        (int)(bounds.x + bounds.width / 2),
        (int)(bounds.y + (bounds.height - fontSize) / 2),
        fontSize,
        TERMINAL_TEXT
    );
}

static void DrawMessage(void)
{
    DrawRectangleRounded(
        (Rectangle){100, 635, 850, 35},
        0.12f,
        6,
        (Color){12, 23, 38, 255}
    );

    DrawRectangleRounded(
        (Rectangle){100, 635, 4, 35},
        0.5f,
        4,
        TERMINAL_CYAN
    );

    DrawFitText(
        message,
        115, 644, 820, 14,
        TERMINAL_TEXT
    );
}

/* Controle das fases */

static void ResetTerminal(void)
{
    phaseIndex = 0;
    stepIndex = 0;
    errorsThisStep = 0;

    introVisible = true;
    gameOver = false;
    gameWon = false;

    SetMessage(
        "Selecione a regra equivalente para continuar."
    );
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

                SetMessage(
                    "Todas as fases foram concluidas!"
                );
            }
            else {
                SetMessage(
                    "Fase superada! Proxima etapa liberada."
                );
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

            SetMessage(
                "Bateria do BIT esgotada."
            );
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
            SetMessage(
                "Resposta incorreta. Bateria reduzida em 20%."
            );
        }
    }
}

/* Funcoes publicas */

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

            SetMessage(
                "Selecione a regra equivalente."
            );
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
    DrawRectangle(
        70, 105, 960, 605,
        TERMINAL_BG
    );

    DrawHeader();

    /* Tela inicial */

    if (introVisible) {
        DrawPanel(
            (Rectangle){170, 240, 760, 330},
            TERMINAL_PANEL
        );

        DrawRectangleRounded(
            (Rectangle){170, 240, 760, 5},
            0.5f,
            6,
            TERMINAL_CYAN
        );

        DrawCentered(
            "NEXORA INDUSTRIES",
            550, 285, 15,
            TERMINAL_CYAN
        );

        DrawCentered(
            "ACESSO AO NUCLEO",
            550, 320, 27,
            TERMINAL_GOLD
        );

        DrawCentered(
            "Simplifique as expressoes usando as regras logicas.",
            550, 375, 17,
            TERMINAL_TEXT
        );

        DrawCentered(
            "Conclua cinco fases com cinco passos cada.",
            550, 410, 15,
            TERMINAL_MUTED
        );

        DrawCentered(
            "Cada erro consome uma vida compartilhada da campanha.",
            550, 440, 15,
            TERMINAL_MUTED
        );

        DrawButton(
            (Rectangle){400, 490, 300, 55},
            "INICIAR TERMINAL"
        );

        return;
    }

    /* Fim de jogo */

    if (gameOver) {
        DrawPanel(
            (Rectangle){220, 270, 660, 220},
            TERMINAL_PANEL
        );

        DrawRectangleRounded(
            (Rectangle){220, 270, 660, 5},
            0.5f,
            6,
            TERMINAL_RED
        );

        DrawCentered(
            "FIM DE JOGO",
            550, 320, 36,
            TERMINAL_RED
        );

        DrawCentered(
            "A bateria de BIT foi esgotada.",
            550, 385, 18,
            TERMINAL_TEXT
        );

        DrawCentered(
            "A campanha nao pode continuar.",
            550, 420, 15,
            TERMINAL_MUTED
        );

        return;
    }

    /* Vitoria */

    if (gameWon) {
        DrawPanel(
            (Rectangle){220, 270, 660, 220},
            TERMINAL_PANEL
        );

        DrawRectangleRounded(
            (Rectangle){220, 270, 660, 5},
            0.5f,
            6,
            TERMINAL_GREEN
        );

        DrawCentered(
            "TERMINAL DESBLOQUEADO!",
            550, 320, 30,
            TERMINAL_GREEN
        );

        DrawCentered(
            "Todas as cinco fases foram concluidas.",
            550, 385, 18,
            TERMINAL_TEXT
        );

        DrawCentered(
            "O nucleo esta acessivel.",
            550, 420, 15,
            TERMINAL_MUTED
        );

        return;
    }

    /* Interface principal */

    const Phase *phase = &phases[phaseIndex];

    DrawProgress();

    DrawPanel(
        (Rectangle){80, 220, 940, 190},
        TERMINAL_PANEL
    );

    DrawText(
        phase->title,
        105, 240, 20,
        TERMINAL_GOLD
    );

    DrawRectangleRounded(
        (Rectangle){105, 275, 110, 27},
        0.3f,
        6,
        (Color){45, 55, 75, 255}
    );

    DrawCentered(
        phase->difficulty,
        160, 282, 13,
        TERMINAL_CYAN
    );

    DrawText(
        TextFormat(
            "PASSO %d / %d",
            stepIndex + 1,
            STEPS_PER_PHASE
        ),
        820, 282, 15,
        TERMINAL_CYAN
    );

    DrawText(
        "EXPRESSAO ATUAL",
        105, 325, 13,
        TERMINAL_MUTED
    );

    DrawRectangleRounded(
        (Rectangle){105, 350, 890, 45},
        0.12f,
        6,
        (Color){10, 24, 39, 255}
    );

    DrawRectangleRoundedLinesEx(
        (Rectangle){105, 350, 890, 45},
        0.12f,
        6,
        1,
        TERMINAL_BORDER
    );

    DrawFitText(
        phase->expressions[stepIndex],
        120, 363, 860, 20,
        TERMINAL_TEXT
    );

    /* Selecao de regras */

    DrawText(
        "ESCOLHA A REGRA EQUIVALENTE",
        100, 435, 16,
        TERMINAL_CYAN
    );

    for (int i = 0; i < RULE_COUNT; i++) {
        DrawButton(
            ruleButtons[i],
            ruleNames[i]
        );
    }

    DrawMessage();

    DrawText(
        "Uma dica sera exibida apos 3 erros no mesmo passo.",
        100, 682, 13,
        TERMINAL_MUTED
    );
}

void Terminal_Unload(void)
{
    campaignLives = NULL;
}
