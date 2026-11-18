#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define W 1100
#define H 760
#define MAX_F 70
#define LEN 96
#define STAGES 5
#define VISIBLE 12
#define STAGE_TIME 180.0f
#define TEXT_SCALE 1.7f

#define DrawText(text, x, y, fontSize, color) \
    DrawTextEx( \
        GetFontDefault(), \
        (text), \
        (Vector2){(float)(x), (float)(y)}, \
        (float)(fontSize) * TEXT_SCALE, \
        1.0f, \
        (color) \
    )

#define MeasureText(text, fontSize) \
    ((int)MeasureTextEx( \
        GetFontDefault(), \
        (text), \
        (float)(fontSize) * TEXT_SCALE, \
        1.0f \
    ).x)

typedef enum {
    AND,
    OR,
    IF,
    IFF
} Op;

typedef struct {
    char text[LEN];
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

static const Stage stages[STAGES] = {
    {
        "LASER DE SEGURANCA",
        "Crie uma formula sempre verdadeira.",
        "Combine p e ~p usando OU.",
        "Uma tautologia e verdadeira em todas as linhas.",
        "O laser se apaga. BIT atravessa o corredor.",
        0x0F
    },
    {
        "TRAVA DE EMERGENCIA",
        "Crie uma formula sempre falsa.",
        "Combine p e ~p usando E.",
        "Uma contradicao e falsa em todas as linhas.",
        "A trava de emergencia foi neutralizada.",
        0x00
    },
    {
        "ELEVADOR DE SERVICO",
        "Recupere o sinal original p.",
        "Selecione ~p e pressione N para nega-la.",
        "Negar duas vezes devolve o valor inicial: ~~p equivale a p.",
        "O elevador desce ao proximo nivel.",
        0x0C
    },
    {
        "FIREWALL DE ACESSO",
        "Crie a implicacao p -> q.",
        "p -> q equivale a ~p OU q.",
        "A implicacao so e falsa quando p e verdadeiro e q e falso.",
        "O firewall foi contornado.",
        0x0B
    },
    {
        "COFRE CENTRAL",
        "Prove que p E q implica p.",
        "Crie p E q e depois use SE... ENTAO com p.",
        "Se p E q sao verdadeiros, p necessariamente e verdadeiro.",
        "O cofre abriu. BIT escapou com os dados.",
        0x0F
    }
};

static const char *opNames[] = {
    "E",
    "OU",
    "SE... ENTAO",
    "SE E SO SE"
};

static const char *opSymbols[] = {
    "^",
    "v",
    "->",
    "<->"
};

static const Color BG        = {6, 10, 19, 255};
static const Color HEADER    = {11, 19, 32, 255};
static const Color PANEL     = {17, 29, 47, 255};
static const Color PANEL2    = {10, 18, 30, 255};
static const Color CARD      = {23, 38, 60, 255};
static const Color HOVER     = {33, 52, 77, 255};
static const Color SELECTED  = {17, 62, 73, 255};
static const Color BORDER    = {49, 73, 101, 255};
static const Color TEXT      = {237, 244, 251, 255};
static const Color MUTED     = {145, 168, 193, 255};
static const Color CYAN      = {64, 220, 230, 255};
static const Color UI_GREEN  = {92, 228, 151, 255};
static const Color UI_RED    = {255, 91, 105, 255};
static const Color UI_GOLD   = {255, 200, 96, 255};
static const Color UI_PURPLE = {181, 137, 255, 255};

static Formula formulas[MAX_F];

static int count;
static int stage;
static int aSel = -1;
static int bSel = -1;
static int opSel = -1;
static int scroll;
static int fusions;
static int discoveries;
static int errors;

static float timeLeft;
static float pressure;
static float flash;

static bool tracker;
static bool hintOn;
static bool solved;
static bool over;
static bool won;
static bool intro = true;

static char statusTitle[96] = "BIT ESTA PRONTO";
static char statusMessage[220] = "Selecione formulas para comecar.";
static Color statusColor = {64, 220, 230, 255};

static void Status(const char *title, const char *message, Color color)
{
    snprintf(statusTitle, sizeof(statusTitle), "%s", title);
    snprintf(statusMessage, sizeof(statusMessage), "%s", message);
    statusColor = color;
}

static unsigned char Eval(unsigned char x, unsigned char y, Op op)
{
    unsigned char result = 0;

    for (int i = 0; i < 4; i++) {
        bool p = (x >> i) & 1;
        bool q = (y >> i) & 1;
        bool value = false;

        switch (op) {
            case AND:
                value = p && q;
                break;
            case OR:
                value = p || q;
                break;
            case IF:
                value = !p || q;
                break;
            case IFF:
                value = p == q;
                break;
        }

        if (value)
            result |= (unsigned char)(1u << i);
    }

    return result;
}

static unsigned char NotTable(unsigned char table)
{
    return (unsigned char)(~table) & 0x0F;
}

static bool Exists(const char *text)
{
    for (int i = 0; i < count; i++) {
        if (strcmp(formulas[i].text, text) == 0)
            return true;
    }

    return false;
}

static bool AddFormula(const char *text, unsigned char table)
{
    if (count >= MAX_F || Exists(text))
        return false;

    snprintf(formulas[count].text, LEN, "%s", text);
    formulas[count].table = table;
    count++;
    discoveries++;

    return true;
}

static void AddStart(const char *text, unsigned char table)
{
    if (count >= MAX_F)
        return;

    snprintf(formulas[count].text, LEN, "%s", text);
    formulas[count].table = table;
    count++;
}

static void Reset(void)
{
    count = 0;
    stage = 0;

    aSel = -1;
    bSel = -1;
    opSel = -1;

    scroll = 0;
    fusions = 0;
    discoveries = 0;
    errors = 0;

    timeLeft = STAGE_TIME;
    pressure = 0.0f;
    flash = 0.0f;

    tracker = false;
    hintOn = false;
    solved = false;
    over = false;
    won = false;
    intro = true;

    AddStart("p", 0x0C);
    AddStart("q", 0x0A);
    AddStart("~p", 0x03);
    AddStart("~q", 0x05);

    Status(
        "BIT ESTA PRONTO",
        "O cronometro comeca quando a operacao iniciar.",
        CYAN
    );
}

static void SelectFormula(int index)
{
    if (aSel == index) {
        aSel = bSel;
        bSel = -1;
    } else if (bSel == index) {
        bSel = -1;
    } else if (aSel < 0) {
        aSel = index;
    } else if (bSel < 0) {
        bSel = index;
    } else {
        aSel = bSel;
        bSel = index;
    }
}

static void Solve(void)
{
    solved = true;
    flash = 1.0f;

    Status(
        stages[stage].success,
        stages[stage].lesson,
        UI_GREEN
    );
}

static void Failure(const char *message)
{
    errors++;

    if (errors >= 3 && !hintOn) {
        hintOn = true;

        Status(
            "BIT: TENHO UMA PISTA",
            stages[stage].hint,
            UI_GOLD
        );
    } else {
        Status(
            "TENTATIVA NAO FUNCIONOU",
            message,
            UI_RED
        );
    }
}

static void Fuse(void)
{
    if (over || won || solved)
        return;

    if (aSel < 0 || bSel < 0 || opSel < 0) {
        Status(
            "FUSAO INCOMPLETA",
            "Selecione duas formulas e um conectivo.",
            UI_GOLD
        );
        return;
    }

    Formula a = formulas[aSel];
    Formula b = formulas[bSel];
    char result[LEN];

    snprintf(
        result,
        sizeof(result),
        "(%s %s %s)",
        a.text,
        opSymbols[opSel],
        b.text
    );

    if (Exists(result)) {
        Failure("Essa fusao ja esta no caderno.");
        return;
    }

    unsigned char table = Eval(a.table, b.table, (Op)opSel);

    if (!AddFormula(result, table)) {
        Status(
            "CADERNO CHEIO",
            "Nao ha espaco para novas formulas.",
            UI_RED
        );
        return;
    }

    fusions++;
    tracker = true;
    errors = 0;
    hintOn = false;

    if (table == stages[stage].target) {
        Solve();
    } else if (table == 0x0F) {
        Status(
            "TAUTOLOGIA DESCOBERTA",
            "A formula e verdadeira em todas as linhas.",
            UI_GREEN
        );
    } else if (table == 0x00) {
        Status(
            "CONTRADICAO DESCOBERTA",
            "A formula e falsa em todas as linhas.",
            UI_GOLD
        );
    } else {
        Status(
            "NOVA FORMULA",
            "Confira a tabela-verdade e tente combina-la.",
            CYAN
        );
    }
}

static void Negate(void)
{
    if (over || won || solved)
        return;

    int index = bSel >= 0 ? bSel : aSel;

    if (index < 0) {
        Status(
            "SELECIONE UMA FORMULA",
            "Clique em uma formula e pressione N.",
            UI_GOLD
        );
        return;
    }

    Formula source = formulas[index];
    char result[LEN];

    snprintf(result, sizeof(result), "~(%s)", source.text);

    if (Exists(result)) {
        Failure("Essa negacao ja esta no caderno.");
        return;
    }

    unsigned char table = NotTable(source.table);

    if (!AddFormula(result, table)) {
        Status(
            "CADERNO CHEIO",
            "Nao ha espaco para novas formulas.",
            UI_RED
        );
        return;
    }

    fusions++;
    tracker = true;
    errors = 0;
    hintOn = false;

    if (table == stages[stage].target) {
        Solve();
    } else {
        Status(
            "NEGACAO DESCOBERTA",
            "Negar inverte verdadeiro e falso em cada linha.",
            CYAN
        );
    }
}

static void NextStage(void)
{
    if (!solved)
        return;

    if (stage == STAGES - 1) {
        won = true;
        return;
    }

    stage++;
    solved = false;

    aSel = -1;
    bSel = -1;
    opSel = -1;

    scroll = 0;
    errors = 0;
    hintOn = false;

    timeLeft = STAGE_TIME;
    pressure = 0.0f;

    Status(
        "NOVA TRAVA LOCALIZADA",
        "O cronometro foi reiniciado para esta fase.",
        CYAN
    );
}

static bool Hover(Rectangle r)
{
    return CheckCollisionPointRec(GetMousePosition(), r);
}

static bool Clicked(Rectangle r)
{
    return Hover(r) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

static void Panel(Rectangle r, Color color)
{
    DrawRectangleRounded(r, 0.06f, 8, color);
    DrawRectangleRoundedLinesEx(r, 0.06f, 8, 1.0f, BORDER);
}

static void Center(const char *text, int x, int y, int size, Color color)
{
    DrawText(
        text,
        x - MeasureText(text, size) / 2,
        y,
        size,
        color
    );
}

static void DrawWrappedText(
    const char *text,
    Rectangle bounds,
    int fontSize,
    float spacing,
    Color color
)
{
    Font font = GetFontDefault();
    float scaledFontSize = (float)fontSize * TEXT_SCALE;
    float scaledSpacing = spacing * TEXT_SCALE;
    float lineHeight = scaledFontSize + 7.0f;
    float x = bounds.x;
    float y = bounds.y;
    const char *word = text;

    while (*word != '\0' && y + lineHeight <= bounds.y + bounds.height) {
        const char *wordEnd = word;

        while (*wordEnd != '\0' && *wordEnd != ' ')
            wordEnd++;

        int wordLength = (int)(wordEnd - word);
        char wordBuffer[256];

        if (wordLength >= (int)sizeof(wordBuffer))
            wordLength = (int)sizeof(wordBuffer) - 1;

        memcpy(wordBuffer, word, (size_t)wordLength);
        wordBuffer[wordLength] = '\0';

        float wordWidth = MeasureTextEx(
            font,
            wordBuffer,
            scaledFontSize,
            scaledSpacing
        ).x;

        float spaceWidth = MeasureTextEx(
            font,
            " ",
            scaledFontSize,
            scaledSpacing
        ).x;

        if (x > bounds.x && x + wordWidth > bounds.x + bounds.width) {
            x = bounds.x;
            y += lineHeight;
        }

        DrawTextEx(
            font,
            wordBuffer,
            (Vector2){x, y},
            scaledFontSize,
            scaledSpacing,
            color
        );

        x += wordWidth;

        if (*wordEnd == ' ') {
            x += spaceWidth;
            word = wordEnd + 1;
        } else {
            word = wordEnd;
        }
    }
}

static void DrawHeader(void)
{
    DrawRectangle(0, 0, W, 78, HEADER);
    DrawRectangle(0, 0, 6, 78, CYAN);

    DrawText("LOGIC CORE", 27, 10, 10, CYAN);
    DrawText("A FUGA DE BIT", 27, 27, 22, TEXT);
    DrawText("INVASAO LOGICA // ACESSO RESTRITO", 28, 54, 9, MUTED);

    int totalSeconds = (int)timeLeft;

    DrawText(
        TextFormat("TEMPO: %02d:%02d", totalSeconds / 60, totalSeconds % 60),
        812,
        10,
        12,
        timeLeft <= 30.0f ? UI_RED : TEXT
    );

    DrawText(
        tracker ? "RASTREADOR ATIVO" : "RASTREADOR INATIVO",
        812,
        27,
        9,
        tracker ? UI_GOLD : MUTED
    );

    Rectangle bar = {812, 47, 232, 12};
    DrawRectangleRounded(bar, 0.5f, 8, PANEL2);

    Color barColor = pressure < 55.0f
        ? CYAN
        : pressure < 80.0f ? UI_GOLD : UI_RED;

    if (pressure > 0.0f) {
        DrawRectangleRounded(
            (Rectangle){812, 47, 232.0f * pressure / 100.0f, 12},
            0.5f,
            8,
            barColor
        );
    }

    DrawText(TextFormat("%d%%", (int)pressure), 1052, 48, 10, TEXT);
    DrawLine(0, 77, W, 77, BORDER);
}

static void DrawTarget(void)
{
    Panel((Rectangle){28, 94, 1044, 129}, PANEL);

    DrawText(
        TextFormat("ALVO %02d / %02d", stage + 1, STAGES),
        48,
        106,
        9,
        CYAN
    );

    DrawText(stages[stage].name, 48, 125, 19, UI_GOLD);
    DrawText(stages[stage].goal, 48, 152, 11, TEXT);

    DrawText(
        hintOn
            ? TextFormat("BIT: %s", stages[stage].hint)
            : "BIT libera uma pista depois de 3 erros.",
        48,
        179,
        9,
        hintOn ? UI_GOLD : MUTED
    );

    DrawCircle(897, 157, 38, PANEL2);
    DrawCircleLines(897, 157, 39, BORDER);

    Color lock = solved
        ? UI_GREEN
        : timeLeft <= 30.0f ? UI_RED : CYAN;

    DrawRectangleRounded((Rectangle){877, 152, 40, 29}, 0.12f, 5, lock);
    DrawCircleLines(897, 149, 13, lock);
    DrawCircle(897, 160, 3, BG);

    DrawText(
        solved ? "ABERTO" : "TRAVA ATIVA",
        866,
        205,
        8,
        solved ? UI_GREEN : MUTED
    );

    for (int i = 0; i < STAGES; i++) {
        Color dot = i < stage ? UI_GREEN : i == stage ? UI_GOLD : BORDER;
        DrawCircle(962 + (i % 3) * 25, 112 + (i / 3) * 28, 5, dot);
    }
}

static void DrawInventory(void)
{
    Rectangle panel = {28, 239, 397, 474};

    /* Corrigido: Panel recebe o retângulo e a cor. */
    Panel(panel, PANEL);

    DrawText("CADERNO DE BIT", 47, 255, 14, TEXT);
    DrawText("Role o mouse para ver todas as formulas.", 47, 277, 9, MUTED);
    DrawLine(47, 295, 406, 295, BORDER);

    int maxScroll = count - VISIBLE;

    if (maxScroll < 0)
        maxScroll = 0;

    if (scroll < 0)
        scroll = 0;

    if (scroll > maxScroll)
        scroll = maxScroll;

    /*
     * São exibidas até 12 fórmulas por vez.
     * A variável scroll indica o índice da primeira fórmula mostrada.
     */
    for (int i = 0; i < VISIBLE && scroll + i < count; i++) {
        int index = scroll + i;
        int row = i / 3;
        int column = i % 3;

        Rectangle r = {
            47.0f + column * 119.0f,
            307.0f + row * 75.0f,
            111.0f,
            67.0f
        };

        bool selected = index == aSel || index == bSel;

        DrawRectangleRounded(
            r,
            0.08f,
            5,
            selected ? SELECTED : Hover(r) ? HOVER : CARD
        );

        DrawRectangleRoundedLinesEx(
            r,
            0.08f,
            5,
            1.0f,
            selected ? CYAN : BORDER
        );

        DrawText(
            TextFormat("%02d", index + 1),
            (int)r.x + 7,
            (int)r.y + 5,
            8,
            MUTED
        );

        int formulaFontSize = 13;

        while (MeasureText(formulas[index].text, formulaFontSize) > r.width - 14 &&
               formulaFontSize > 6) {
            formulaFontSize--;
        }

        DrawText(
            formulas[index].text,
            (int)r.x + 7,
            (int)r.y + 23,
            formulaFontSize,
            TEXT
        );

        Color tag = formulas[index].table == 0x0F
            ? UI_GREEN
            : formulas[index].table == 0x00 ? UI_RED : MUTED;

        DrawText(
            formulas[index].table == 0x0F
                ? "SEMPRE V"
                : formulas[index].table == 0x00 ? "SEMPRE F" : "VARIA",
            (int)r.x + 7,
            (int)r.y + 46,
            8,
            tag
        );

        if (Clicked(r))
            SelectFormula(index);
    }

    DrawText(
        TextFormat("%d / %d FORMULAS", count, MAX_F),
        47,
        677,
        9,
        CYAN
    );

    DrawText(
        TextFormat("FUSOES: %d  DESCOBERTAS: %d", fusions, discoveries),
        47,
        695,
        8,
        MUTED
    );

    Rectangle track = {405, 307, 8, 292};
    DrawRectangleRounded(track, 0.5f, 4, PANEL2);

    if (count > VISIBLE) {
        float thumbHeight = track.height * (float)VISIBLE / (float)count;

        if (thumbHeight < 24.0f)
            thumbHeight = 24.0f;

        float scrollRange = track.height - thumbHeight;
        float scrollRatio = maxScroll > 0
            ? (float)scroll / (float)maxScroll
            : 0.0f;

        Rectangle thumb = {
            track.x,
            track.y + scrollRange * scrollRatio,
            track.width,
            thumbHeight
        };

        DrawRectangleRounded(thumb, 0.5f, 4, CYAN);

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
            CheckCollisionPointRec(GetMousePosition(), track) &&
            scrollRange > 0.0f) {
            float relativeY =
                GetMousePosition().y - track.y - thumbHeight / 2.0f;

            if (relativeY < 0.0f)
                relativeY = 0.0f;

            if (relativeY > scrollRange)
                relativeY = scrollRange;

            scroll = (int)(
                (relativeY / scrollRange) * (float)maxScroll
            );
        }
    }

    if (Hover(panel) && count > VISIBLE) {
        float wheel = GetMouseWheelMove();

        if (wheel != 0.0f)
            scroll -= (int)(wheel * 3.0f);

        if (scroll < 0)
            scroll = 0;

        if (scroll > maxScroll)
            scroll = maxScroll;
    } else {
        GetMouseWheelMove();
    }
}

static void DrawControls(void)
{
    Panel((Rectangle){443, 239, 629, 242}, PANEL);

    DrawText("MESA DE FUSAO", 466, 255, 15, TEXT);
    DrawText("Combine duas formulas com um conectivo.", 466, 277, 9, MUTED);
    DrawLine(466, 296, 1049, 296, BORDER);

    Panel((Rectangle){466, 307, 220, 43}, PANEL2);
    Panel((Rectangle){829, 307, 220, 43}, PANEL2);

    DrawText(
        aSel >= 0 ? formulas[aSel].text : "FORMULA A",
        477,
        321,
        12,
        aSel >= 0 ? TEXT : MUTED
    );

    DrawText(
        bSel >= 0 ? formulas[bSel].text : "FORMULA B",
        840,
        321,
        12,
        bSel >= 0 ? TEXT : MUTED
    );

    Center("+", 757, 315, 23, UI_PURPLE);

    for (int i = 0; i < 4; i++) {
        Rectangle r = {466 + i * 147, 362, 140, 34};
        bool selected = opSel == i;

        DrawRectangleRounded(
            r,
            0.08f,
            5,
            selected ? (Color){51, 37, 77, 255}
                     : Hover(r) ? HOVER : CARD
        );

        DrawRectangleRoundedLinesEx(
            r,
            0.08f,
            5,
            1,
            selected ? UI_PURPLE : BORDER
        );

        Center(
            opNames[i],
            (int)(r.x + r.width / 2),
            (int)r.y + 11,
            8,
            selected ? UI_PURPLE : TEXT
        );

        if (Clicked(r))
            opSel = i;
    }

    Rectangle fuse = {466, 410, 367, 39};

    DrawRectangleRounded(fuse, 0.08f, 5, SELECTED);
    DrawRectangleRoundedLinesEx(fuse, 0.08f, 5, 1.5f, CYAN);
    Center("EXECUTAR FUSAO", 649, 422, 11, CYAN);

    if (Clicked(fuse))
        Fuse();

    Rectangle negate = {846, 410, 203, 39};

    DrawRectangleRounded(negate, 0.08f, 5, CARD);
    DrawRectangleRoundedLinesEx(negate, 0.08f, 5, 1, BORDER);
    Center("NEGAR FORMULA [N]", 947, 423, 8, UI_PURPLE);

    if (Clicked(negate))
        Negate();

    DrawText("O cronometro continua durante a fase.", 466, 459, 8, MUTED);
}

static void DrawAnalysis(void)
{
    Panel((Rectangle){443, 496, 629, 217}, PANEL);

    DrawText("ANALISE DA FORMULA", 466, 511, 13, TEXT);
    DrawLine(466, 532, 1049, 532, BORDER);

    DrawText(statusTitle, 466, 540, 9, statusColor);
    DrawText(statusMessage, 466, 557, 9, TEXT);

    int index = bSel >= 0 ? bSel : aSel;

    if (index < 0 || index >= count) {
        DrawText("Selecione uma formula para analisar.", 466, 606, 9, MUTED);
        return;
    }

    Formula f = formulas[index];

    DrawText(TextFormat("FORMULA: %s", f.text), 466, 590, 9, CYAN);

    const char *rows[] = {
        "p=F q=F",
        "p=F q=V",
        "p=V q=F",
        "p=V q=V"
    };

    for (int i = 0; i < 4; i++) {
        bool value = (f.table >> i) & 1;
        int x = 466 + i * 143;

        DrawRectangleRounded(
            (Rectangle){x, 610, 132, 40},
            0.08f,
            4,
            PANEL2
        );

        DrawText(rows[i], x + 7, 615, 8, MUTED);

        DrawText(
            value ? "VERDADEIRO" : "FALSO",
            x + 7,
            630,
            8,
            value ? UI_GREEN : UI_RED
        );
    }

    DrawText(
        f.table == 0x0F
            ? "TAUTOLOGIA: sempre verdadeira."
            : f.table == 0x00
                ? "CONTRADICAO: sempre falsa."
                : "CONTINGENTE: depende dos valores.",
        466,
        670,
        8,
        f.table == 0x0F ? UI_GREEN
                       : f.table == 0x00 ? UI_RED : UI_GOLD
    );
}

static void Overlay(
    const char *title,
    const char *mainText,
    const char *detail,
    const char *button,
    Color accent
)
{
    DrawRectangle(0, 0, W, H, (Color){2, 5, 12, 215});
    Panel((Rectangle){250, 225, 600, 300}, PANEL);

    Center(title, W / 2, 257, 14, accent);
    Center(mainText, W / 2, 294, 20, TEXT);
    Center(detail, W / 2, 338, 10, MUTED);

    Rectangle r = {400, 412, 300, 46};

    DrawRectangleRounded(r, 0.1f, 6, SELECTED);
    DrawRectangleRoundedLinesEx(r, 0.1f, 6, 2, accent);
    Center(button, W / 2, 428, 11, TEXT);

    if (Clicked(r)) {
        if (over || won)
            Reset();
        else
            NextStage();
    }
}

static void DrawIntro(void)
{
    DrawRectangle(0, 0, W, H, (Color){2, 5, 12, 220});
    Panel((Rectangle){250, 225, 600, 300}, PANEL);

    Center("MISSAO: FUGA DE BIT", W / 2, 255, 20, CYAN);
    Center("Combine formulas para superar as travas.", W / 2, 300, 12, TEXT);

    Center(
        "Selecione duas formulas, escolha um conectivo e funda.",
        W / 2,
        330,
        10,
        MUTED
    );

    Center("Pressione N para negar uma formula.", W / 2, 352, 10, MUTED);

    Center(
        "Voce tem 3 minutos para cada fase. O tempo reinicia ao avancar.",
        W / 2,
        374,
        10,
        UI_GREEN
    );

    Rectangle r = {400, 420, 300, 46};

    DrawRectangleRounded(r, 0.1f, 6, SELECTED);
    DrawRectangleRoundedLinesEx(r, 0.1f, 6, 2, CYAN);
    Center("COMECAR", W / 2, 435, 12, TEXT);

    if (Clicked(r))
        intro = false;
}

int main(void)
{
    InitWindow(W, H, "LOGIC CORE | A Fuga de BIT");
    SetTargetFPS(60);

    Reset();

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        if (!intro && IsKeyPressed(KEY_R))
            Reset();

        if (!intro && !over && !won && !solved) {
            timeLeft -= dt;

            if (timeLeft <= 0.0f) {
                timeLeft = 0.0f;
                pressure = 100.0f;
                over = true;

                Status(
                    "TEMPO ESGOTADO",
                    "BIT nao conseguiu superar a trava a tempo.",
                    UI_RED
                );
            }

            if (!over && IsKeyPressed(KEY_N))
                Negate();

            if (!over && IsKeyPressed(KEY_ENTER))
                Fuse();
        }

        if (!intro && !over && !won && !solved) {
            pressure = 100.0f * (1.0f - timeLeft / STAGE_TIME);

            if (pressure < 0.0f)
                pressure = 0.0f;

            if (pressure > 100.0f)
                pressure = 100.0f;
        }

        if (flash > 0.0f) {
            flash -= dt;

            if (flash < 0.0f)
                flash = 0.0f;
        }

        BeginDrawing();
        ClearBackground(BG);

        DrawHeader();
        DrawTarget();
        DrawInventory();
        DrawControls();
        DrawAnalysis();

        DrawText(
            "N: negar   ENTER: fundir   R: reiniciar",
            28,
            735,
            9,
            MUTED
        );

        if (flash > 0.0f) {
            DrawRectangle(
                0,
                78,
                W,
                H - 78,
                (Color){91, 228, 151, (unsigned char)(flash * 55)}
            );
        }

        if (solved && !won) {
            Overlay(
                "TRAVA SUPERADA",
                stages[stage].success,
                stages[stage].lesson,
                stage + 1 < STAGES
                    ? "PROXIMO DESAFIO"
                    : "ABRIR O COFRE",
                UI_GREEN
            );
        }

        if (over) {
            Overlay(
                "TEMPO ESGOTADO",
                "O tempo desta fase acabou.",
                "Reinicie a operacao para tentar novamente.",
                "REINICIAR",
                UI_RED
            );
        }

        if (won) {
            Overlay(
                "OPERACAO CONCLUIDA",
                "BIT escapou com os dados.",
                TextFormat(
                    "%d fusoes e %d formulas descobertas.",
                    fusions,
                    discoveries
                ),
                "JOGAR NOVAMENTE",
                UI_GOLD
            );
        }

        if (intro)
            DrawIntro();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}