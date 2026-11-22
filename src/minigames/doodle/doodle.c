
#include "doodle.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define MAX_FORMULAS 70
#define FORMULA_LENGTH 96
#define TOTAL_STAGES 5
#define VISIBLE_FORMULAS 12
#define STAGE_TIME 180.0f

typedef enum {
    OP_AND,
    OP_OR,
    OP_IF,
    OP_IFF
} Operator;

typedef struct {
    char text[FORMULA_LENGTH];
    unsigned char table;
} Formula;

typedef struct {
    const char *name;
    const char *goal;
    const char *hint;
    const char *lesson;
    const char *success;
    unsigned char target;
} Stage;

static const Stage stages[TOTAL_STAGES] = {
    {
        "LASER DE SEGURANCA",
        "Crie uma formula sempre verdadeira.",
        "Combine p e ~p usando OU.",
        "Uma tautologia e verdadeira em todas as linhas.",
        "O laser foi desativado!",
        0x0F
    },
    {
        "TRAVA DE EMERGENCIA",
        "Crie uma formula sempre falsa.",
        "Combine p e ~p usando E.",
        "Uma contradicao e falsa em todas as linhas.",
        "A trava foi neutralizada!",
        0x00
    },
    {
        "ELEVADOR DE SERVICO",
        "Recupere o sinal original p.",
        "Selecione ~p e pressione N para nega-la.",
        "Negar duas vezes devolve o valor inicial.",
        "O elevador foi liberado!",
        0x0C
    },
    {
        "FIREWALL DE ACESSO",
        "Crie a implicacao p -> q.",
        "p -> q equivale a ~p OU q.",
        "A implicacao so e falsa quando p e verdadeiro e q e falso.",
        "O firewall foi contornado!",
        0x0B
    },
    {
        "COFRE CENTRAL",
        "Prove que p E q implica p.",
        "Crie p E q e depois use SE... ENTAO com p.",
        "Se p e q sao verdadeiros, p necessariamente e verdadeiro.",
        "O cofre abriu. Dados recuperados!",
        0x0F
    }
};

static const char *operatorNames[] = {
    "E",
    "OU",
    "SE... ENTAO",
    "SE E SO SE"
};

static const char *operatorSymbols[] = {
    "^",
    "v",
    "->",
    "<->"
};

/* Paleta visual */

static const Color BG = {12, 16, 27, 255};
static const Color PANEL = {23, 31, 48, 255};
static const Color CARD = {35, 46, 66, 255};
static const Color HOVER = {52, 68, 92, 255};
static const Color SELECTED = {20, 75, 88, 255};
static const Color BORDER = {75, 110, 145, 255};
static const Color TEXT = {240, 245, 252, 255};
static const Color MUTED = {165, 180, 200, 255};
static const Color CYAN = {80, 205, 235, 255};

static const Color DOODLE_GREEN = {85, 220, 145, 255};
static const Color DOODLE_RED = {245, 90, 100, 255};
static const Color DOODLE_GOLD = {255, 200, 90, 255};
static const Color DOODLE_PURPLE = {185, 140, 255, 255};

/* Estado do minigame */

static Formula formulas[MAX_FORMULAS];

static int formulaCount;
static int currentStage;
static int selectedA;
static int selectedB;
static int selectedOperator;
static int scroll;
static int errors;

static int *campaignLives;

static float timeLeft;
static float pressure;

static bool intro;
static bool solved;
static bool hintOn;
static bool tracker;

static char statusTitle[100];
static char statusMessage[200];

static Color statusColor;

static DoodleResult result = DOODLE_RUNNING;

/* Funcoes auxiliares */

static void SetStatus(
    const char *title,
    const char *message,
    Color color
)
{
    snprintf(statusTitle, sizeof(statusTitle), "%s", title);
    snprintf(statusMessage, sizeof(statusMessage), "%s", message);
    statusColor = color;
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

    DrawRectangleRounded(bounds, 0.06f, 6, color);

    DrawRectangleRoundedLinesEx(
        bounds,
        0.06f,
        6,
        1.5f,
        BORDER
    );
}

static void DrawCentered(
    const char *text,
    int centerX,
    int y,
    int size,
    Color color
)
{
    int width = MeasureText(text, size);
    DrawText(text, centerX - width / 2, y, size, color);
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
        fontSize > 8
    ) {
        fontSize--;
    }

    DrawText(text, x, y, fontSize, color);
}

/* Logica das formulas */

static unsigned char Evaluate(
    unsigned char a,
    unsigned char b,
    Operator op
)
{
    unsigned char table = 0;

    for (int i = 0; i < 4; i++) {
        bool p = (a >> i) & 1;
        bool q = (b >> i) & 1;
        bool value = false;

        switch (op) {
            case OP_AND:
                value = p && q;
                break;

            case OP_OR:
                value = p || q;
                break;

            case OP_IF:
                value = !p || q;
                break;

            case OP_IFF:
                value = p == q;
                break;
        }

        if (value)
            table |= (unsigned char)(1u << i);
    }

    return table;
}

static unsigned char NegateTable(unsigned char table)
{
    return (unsigned char)(~table) & 0x0F;
}

static bool FormulaExists(const char *text)
{
    for (int i = 0; i < formulaCount; i++) {
        if (strcmp(formulas[i].text, text) == 0)
            return true;
    }

    return false;
}

static bool AddFormula(
    const char *text,
    unsigned char table
)
{
    if (
        formulaCount >= MAX_FORMULAS ||
        FormulaExists(text)
    ) {
        return false;
    }

    snprintf(
        formulas[formulaCount].text,
        FORMULA_LENGTH,
        "%s",
        text
    );

    formulas[formulaCount].table = table;
    formulaCount++;

    return true;
}

/* Controle das fases */

static void ResetStage(void)
{
    formulaCount = 0;

    selectedA = -1;
    selectedB = -1;
    selectedOperator = -1;

    scroll = 0;
    errors = 0;

    hintOn = false;
    tracker = false;
    solved = false;

    timeLeft = STAGE_TIME;
    pressure = 0;

    AddFormula("p", 0x0C);
    AddFormula("q", 0x0A);
    AddFormula("~p", 0x03);
    AddFormula("~q", 0x05);

    SetStatus(
        "BIT ESTA PRONTO",
        "Selecione formulas para comecar.",
        CYAN
    );
}

static void SelectFormula(int index)
{
    if (selectedA == index) {
        selectedA = selectedB;
        selectedB = -1;
    }
    else if (selectedB == index) {
        selectedB = -1;
    }
    else if (selectedA < 0) {
        selectedA = index;
    }
    else if (selectedB < 0) {
        selectedB = index;
    }
    else {
        selectedA = selectedB;
        selectedB = index;
    }
}

static void SolveStage(void)
{
    solved = true;

    SetStatus(
        "DESAFIO SUPERADO",
        stages[currentStage].lesson,
        DOODLE_GREEN
    );
}

static void ReportFailure(const char *message)
{
    errors++;

    if (campaignLives != NULL) {
        *campaignLives -= 20;

        if (*campaignLives < 0)
            *campaignLives = 0;

        if (*campaignLives <= 0) {
            result = DOODLE_GAME_OVER;
            return;
        }
    }

    if (errors >= 3 && !hintOn) {
        hintOn = true;

        SetStatus(
            "BIT: TENHO UMA PISTA",
            stages[currentStage].hint,
            DOODLE_GOLD
        );
    }
    else {
        SetStatus(
            "TENTATIVA INCORRETA",
            message,
            DOODLE_RED
        );
    }
}

/* Operacoes do jogador */

static void FuseFormulas(void)
{
    if (solved || result != DOODLE_RUNNING)
        return;

    if (
        selectedA < 0 ||
        selectedB < 0 ||
        selectedOperator < 0
    ) {
        SetStatus(
            "FUSAO INCOMPLETA",
            "Selecione duas formulas e um conectivo.",
            DOODLE_GOLD
        );

        return;
    }

    Formula a = formulas[selectedA];
    Formula b = formulas[selectedB];

    char newFormula[FORMULA_LENGTH];

    snprintf(
        newFormula,
        sizeof(newFormula),
        "(%s %s %s)",
        a.text,
        operatorSymbols[selectedOperator],
        b.text
    );

    if (FormulaExists(newFormula)) {
        ReportFailure("Essa formula ja existe no caderno.");
        return;
    }

    unsigned char table = Evaluate(
        a.table,
        b.table,
        (Operator)selectedOperator
    );

    if (!AddFormula(newFormula, table)) {
        SetStatus(
            "CADERNO CHEIO",
            "Nao ha espaco para novas formulas.",
            DOODLE_RED
        );

        return;
    }

    tracker = true;
    errors = 0;
    hintOn = false;

    if (table == stages[currentStage].target) {
        SolveStage();
    }
    else if (table == 0x0F) {
        SetStatus(
            "TAUTOLOGIA DESCOBERTA",
            "A formula e verdadeira em todas as linhas.",
            DOODLE_GREEN
        );
    }
    else if (table == 0x00) {
        SetStatus(
            "CONTRADICAO DESCOBERTA",
            "A formula e falsa em todas as linhas.",
            DOODLE_GOLD
        );
    }
    else {
        SetStatus(
            "NOVA FORMULA",
            "Confira a tabela-verdade e tente combina-la.",
            CYAN
        );
    }
}

static void NegateFormula(void)
{
    if (solved || result != DOODLE_RUNNING)
        return;

    int index = selectedB >= 0 ? selectedB : selectedA;

    if (index < 0) {
        SetStatus(
            "SELECIONE UMA FORMULA",
            "Clique em uma formula e pressione N.",
            DOODLE_GOLD
        );

        return;
    }

    Formula source = formulas[index];

    char newFormula[FORMULA_LENGTH];

    snprintf(
        newFormula,
        sizeof(newFormula),
        "~(%s)",
        source.text
    );

    if (FormulaExists(newFormula)) {
        ReportFailure("Essa negacao ja existe no caderno.");
        return;
    }

    unsigned char table = NegateTable(source.table);

    if (!AddFormula(newFormula, table)) {
        SetStatus(
            "CADERNO CHEIO",
            "Nao ha espaco para novas formulas.",
            DOODLE_RED
        );

        return;
    }

    errors = 0;
    hintOn = false;
    tracker = true;

    if (table == stages[currentStage].target) {
        SolveStage();
    }
    else {
        SetStatus(
            "NEGACAO DESCOBERTA",
            "Negar inverte verdadeiro e falso.",
            CYAN
        );
    }
}

static void AdvanceStage(void)
{
    if (!solved)
        return;

    if (currentStage == TOTAL_STAGES - 1) {
        result = DOODLE_COMPLETED;
        return;
    }

    currentStage++;
    ResetStage();
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
        CYAN
    );

    DrawText(
        "DOODLE LOGIC",
        95, 117, 27,
        CYAN
    );

    DrawText(
        "MESA DE OPERACOES LOGICAS",
        97, 143, 10,
        MUTED
    );

    DrawRectangleRounded(
        (Rectangle){750, 115, 145, 34},
        0.3f, 8,
        (Color){30, 45, 65, 255}
    );

    DrawCentered(
        TextFormat(
            "FASE %d / %d",
            currentStage + 1,
            TOTAL_STAGES
        ),
        822,
        125,
        16,
        WHITE
    );

    DrawRectangleRounded(
        (Rectangle){910, 115, 100, 34},
        0.3f, 8,
        (Color){25, 55, 48, 255}
    );

    DrawCentered(
        TextFormat(
            "VIDAS: %d",
            campaignLives ? *campaignLives : 0
        ),
        960,
        125,
        15,
        DOODLE_GREEN
    );
}

static void DrawObjective(void)
{
    Rectangle panel = {80, 175, 940, 105};

    DrawPanel(panel, PANEL);

    DrawRectangleRounded(
        (Rectangle){98, 190, 5, 72},
        0.5f, 4,
        DOODLE_GOLD
    );

    DrawText(
        "OBJETIVO DA MISSAO",
        118, 190, 13,
        CYAN
    );

    DrawText(
        stages[currentStage].name,
        118, 210, 20,
        DOODLE_GOLD
    );

    DrawFitText(
        stages[currentStage].goal,
        118, 237, 850, 17,
        TEXT
    );

    DrawFitText(
        hintOn
            ? TextFormat("PISTA DO BIT: %s", stages[currentStage].hint)
            : "BIT oferece uma pista depois de 3 erros.",
        118, 258, 850, 12,
        hintOn ? DOODLE_GOLD : MUTED
    );
}

static void DrawInventory(void)
{
    Rectangle panel = {80, 295, 430, 340};

    DrawPanel(panel, PANEL);

    DrawText(
        "CADERNO DE FORMULAS",
        100, 310, 19,
        CYAN
    );

    DrawText(
        "Selecione duas formulas para combinar.",
        100, 337, 13,
        MUTED
    );

    DrawRectangle(
        100, 355, 390, 2,
        (Color){50, 75, 100, 255}
    );

    int maxScroll = formulaCount - VISIBLE_FORMULAS;

    if (maxScroll < 0)
        maxScroll = 0;

    if (scroll < 0)
        scroll = 0;

    if (scroll > maxScroll)
        scroll = maxScroll;

    for (
        int i = 0;
        i < VISIBLE_FORMULAS && scroll + i < formulaCount;
        i++
    ) {
        int index = scroll + i;
        int row = i / 3;
        int column = i % 3;

        Rectangle button = {
            100 + column * 130,
            365 + row * 57,
            120,
            48
        };

        bool selected =
            index == selectedA || index == selectedB;

        Color cardColor = selected
            ? (Color){18, 75, 91, 255}
            : Hover(button)
                ? (Color){48, 65, 88, 255}
                : CARD;

        DrawRectangleRounded(
            (Rectangle){
                button.x + 2,
                button.y + 3,
                button.width,
                button.height
            },
            0.12f, 6,
            (Color){0, 0, 0, 55}
        );

        DrawRectangleRounded(
            button,
            0.12f, 6,
            cardColor
        );

        DrawRectangleRoundedLinesEx(
            button,
            0.12f, 6,
            selected ? 2 : 1,
            selected ? CYAN : BORDER
        );

        DrawCentered(
            formulas[index].text,
            (int)(button.x + button.width / 2),
            (int)button.y + 15,
            13,
            selected ? CYAN : TEXT
        );

        if (Clicked(button))
            SelectFormula(index);
    }

    DrawRectangle(
        100, 590, 390, 2,
        (Color){50, 75, 100, 255}
    );

    DrawText(
        TextFormat(
            "%d FORMULAS DESCOBERTAS",
            formulaCount
        ),
        100, 600, 13,
        CYAN
    );

    if (Hover(panel) && formulaCount > VISIBLE_FORMULAS) {
        float wheel = GetMouseWheelMove();

        if (wheel != 0)
            scroll -= (int)(wheel * 2);
    }
    else {
        GetMouseWheelMove();
    }
}

static void DrawControls(void)
{
    DrawPanel(
        (Rectangle){530, 295, 490, 340},
        PANEL
    );

    DrawText(
        "MESA DE FUSAO",
        555, 310, 20,
        CYAN
    );

    DrawText(
        "Combine duas formulas usando um conectivo.",
        555, 337, 13,
        MUTED
    );

    /* Formula A */

    DrawRectangleRounded(
        (Rectangle){555, 365, 205, 30},
        0.12f, 6,
        (Color){10, 24, 39, 255}
    );

    DrawFitText(
        selectedA >= 0
            ? formulas[selectedA].text
            : "FORMULA A",
        565, 373, 185, 15,
        selectedA >= 0 ? CYAN : MUTED
    );

    /* Formula B */

    DrawRectangleRounded(
        (Rectangle){790, 365, 205, 30},
        0.12f, 6,
        (Color){10, 24, 39, 255}
    );

    DrawFitText(
        selectedB >= 0
            ? formulas[selectedB].text
            : "FORMULA B",
        800, 373, 185, 15,
        selectedB >= 0 ? CYAN : MUTED
    );

    /* Conectivos */

    for (int i = 0; i < 4; i++) {
        Rectangle button = {
            555 + (i % 2) * 220,
            400 + (i / 2) * 48,
            205,
            38
        };

        bool selected = selectedOperator == i;

        Color background = selected
            ? (Color){57, 42, 88, 255}
            : Hover(button) ? HOVER : CARD;

        DrawRectangleRounded(
            button,
            0.12f, 6,
            background
        );

        DrawRectangleRoundedLinesEx(
            button,
            0.12f, 6,
            selected ? 2 : 1,
            selected ? DOODLE_PURPLE : BORDER
        );

        DrawCentered(
            operatorNames[i],
            (int)(button.x + button.width / 2),
            (int)button.y + 11,
            14,
            selected ? DOODLE_PURPLE : TEXT
        );

        if (Clicked(button))
            selectedOperator = i;
    }

    /* Botao fundir */

    Rectangle fuseButton = {555, 510, 205, 45};

    DrawRectangleRounded(
        fuseButton,
        0.12f, 6,
        (Color){17, 78, 94, 255}
    );

    DrawRectangleRoundedLinesEx(
        fuseButton,
        0.12f, 6,
        2,
        CYAN
    );

    DrawCentered(
        "FUNDIR FORMULAS",
        657, 523, 15,
        TEXT
    );

    if (Clicked(fuseButton))
        FuseFormulas();

    /* Botao negar */

    Rectangle negateButton = {790, 510, 205, 45};

    DrawRectangleRounded(
        negateButton,
        0.12f, 6,
        (Color){49, 37, 72, 255}
    );

    DrawRectangleRoundedLinesEx(
        negateButton,
        0.12f, 6,
        2,
        DOODLE_PURPLE
    );

    DrawCentered(
        "NEGAR FORMULA [N]",
        892, 523, 14,
        TEXT
    );

    if (Clicked(negateButton))
        NegateFormula();

    /* Status do BIT */

    DrawRectangleRounded(
        (Rectangle){550, 570, 450, 53},
        0.12f, 6,
        (Color){12, 23, 38, 255}
    );

    DrawRectangleRounded(
        (Rectangle){550, 570, 4, 53},
        0.5f, 4,
        statusColor
    );

    DrawFitText(
        statusTitle,
        565, 578, 420, 14,
        statusColor
    );

    DrawFitText(
        statusMessage,
        565, 600, 420, 13,
        TEXT
    );
}

static void DrawTimer(void)
{
    int seconds = (int)timeLeft;

    DrawText(
        TextFormat(
            "TEMPO: %02d:%02d",
            seconds / 60,
            seconds % 60
        ),
        100, 650, 18,
        timeLeft <= 30 ? DOODLE_RED : TEXT
    );

    DrawRectangleRounded(
        (Rectangle){300, 654, 650, 15},
        0.5f, 8,
        (Color){35, 45, 60, 255}
    );

    float progress = pressure / 100.0f;

    if (progress < 0)
        progress = 0;

    if (progress > 1)
        progress = 1;

    if (progress > 0) {
        DrawRectangleRounded(
            (Rectangle){
                300, 654,
                650 * progress, 15
            },
            0.5f, 8,
            pressure >= 80 ? DOODLE_RED : CYAN
        );
    }

    DrawText(
        tracker ? "RASTREADOR ATIVO" : "RASTREADOR INATIVO",
        100, 685, 14,
        tracker ? DOODLE_GOLD : MUTED
    );

    DrawText(
        "N: NEGAR FORMULA   ENTER: FUNDIR",
        650, 685, 13,
        MUTED
    );
}

static void DrawOverlay(
    const char *title,
    const char *description,
    const char *buttonText,
    Color color
)
{
    DrawRectangle(
        70, 105, 960, 605,
        (Color){2, 5, 12, 220}
    );

    DrawPanel(
        (Rectangle){220, 270, 660, 220},
        PANEL
    );

    DrawRectangleRounded(
        (Rectangle){220, 270, 660, 5},
        0.5f, 6,
        color
    );

    DrawCentered(
        title,
        550, 310, 27,
        color
    );

    int descriptionSize = 17;

    while (
        MeasureText(description, descriptionSize) > 600 &&
        descriptionSize > 10
    ) {
        descriptionSize--;
    }

    DrawCentered(
        description,
        550, 365, descriptionSize,
        TEXT
    );

    Rectangle button = {390, 415, 320, 48};

    bool hover = Hover(button);

    DrawRectangleRounded(
        button,
        0.12f, 6,
        hover
            ? (Color){25, 105, 125, 255}
            : SELECTED
    );

    DrawRectangleRoundedLinesEx(
        button,
        0.12f, 6,
        2,
        color
    );

    DrawCentered(
        buttonText,
        550, 429, 17,
        WHITE
    );

    if (Clicked(button)) {
        if (solved) {
            AdvanceStage();
        }
        else if (intro) {
            intro = false;
        }
    }
}

/* Funcoes publicas do minigame */

void Doodle_Init(int *lives)
{
    campaignLives = lives;

    currentStage = 0;
    result = DOODLE_RUNNING;
    intro = true;

    ResetStage();

    SetStatus(
        "MISSAO: FUGA DE BIT",
        "Combine formulas para superar as travas.",
        CYAN
    );
}

DoodleResult Doodle_Update(float dt)
{
    if (result != DOODLE_RUNNING)
        return result;

    if (campaignLives == NULL || *campaignLives <= 0) {
        result = DOODLE_GAME_OVER;
        return result;
    }

    if (intro || solved)
        return result;

    if (IsKeyPressed(KEY_N))
        NegateFormula();

    if (IsKeyPressed(KEY_ENTER))
        FuseFormulas();

    if (solved)
        return result;

    timeLeft -= dt;

    if (timeLeft <= 0.0f) {
        timeLeft = 0.0f;

        (*campaignLives) -= 20;

        if (*campaignLives < 0)
            *campaignLives = 0;

        if (*campaignLives <= 0) {
            result = DOODLE_GAME_OVER;
            return result;
        }

        ResetStage();

        SetStatus(
            "TEMPO ESGOTADO",
            "Bateria reduzida em 20%. Tente novamente esta fase.",
            DOODLE_RED
        );

        return result;
    }

    pressure = 100.0f * (1.0f - timeLeft / STAGE_TIME);

    if (pressure < 0)
        pressure = 0;

    if (pressure > 100)
        pressure = 100;

    return result;
}

void Doodle_Draw(void)
{
    DrawRectangle(
        70, 105, 960, 605,
        BG
    );

    DrawHeader();

    if (result == DOODLE_COMPLETED) {
        DrawOverlay(
            "OPERACAO CONCLUIDA",
            "BIT superou os cinco desafios!",
            "COFRE ABERTO",
            DOODLE_GREEN
        );

        return;
    }

    if (result == DOODLE_GAME_OVER) {
        DrawOverlay(
            "FIM DE JOGO",
            "As vidas da campanha acabaram.",
            "FIM",
            DOODLE_RED
        );

        return;
    }

    DrawObjective();
    DrawInventory();
    DrawControls();
    DrawTimer();

    if (intro) {
        DrawOverlay(
            "MISSAO: FUGA DE BIT",
            "Selecione formulas, escolha conectivos e funda.",
            "COMECAR",
            CYAN
        );
    }
    else if (solved) {
        DrawOverlay(
            "TRAVA SUPERADA",
            stages[currentStage].success,
            currentStage == TOTAL_STAGES - 1
                ? "ABRIR O COFRE"
                : "PROXIMO DESAFIO",
            DOODLE_GREEN
        );
    }
}

void Doodle_Unload(void)
{
    campaignLives = NULL;
}
