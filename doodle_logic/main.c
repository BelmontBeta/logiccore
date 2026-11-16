#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define W 1100
#define H 760
#define MAX_F 70
#define LEN 96
#define STAGES 5
#define VISIBLE 8

typedef enum { AND, OR, IF, IFF } Op;

typedef struct {
    char text[LEN];
    unsigned char table;
} Formula;

typedef struct {
    const char *name, *goal, *hint, *lesson, *success;
    unsigned char target;
} Stage;

static const Stage stages[STAGES] = {
    {"LASER DE SEGURANCA", "Crie uma formula sempre verdadeira.",
     "Combine p e ~p usando OU.",
     "Uma tautologia e verdadeira em todas as linhas.",
     "O laser se apaga. BIT atravessa o corredor.", 0x0F},

    {"TRAVA DE EMERGENCIA", "Crie uma formula sempre falsa.",
     "Combine p e ~p usando E.",
     "Uma contradicao e falsa em todas as linhas.",
     "A trava de emergencia foi neutralizada.", 0x00},

    {"ELEVADOR DE SERVICO", "Recupere o sinal original p.",
     "Selecione ~p e pressione N para nega-la.",
     "Negar duas vezes devolve o valor inicial: ~~p equivale a p.",
     "O elevador desce ao proximo nivel.", 0x0C},

    {"FIREWALL DE ACESSO", "Crie a implicacao p -> q.",
     "p -> q equivale a ~p OU q.",
     "A implicacao so e falsa quando p e verdadeiro e q e falso.",
     "O firewall foi contornado.", 0x0B},

    {"COFRE CENTRAL", "Prove que p E q implica p.",
     "Crie p E q e depois use SE... ENTAO com p.",
     "Se p E q sao verdadeiros, p necessariamente e verdadeiro.",
     "O cofre abriu. BIT escapou com os dados.", 0x0F}
};

static const char *opNames[] = {"E", "OU", "SE... ENTAO", "SE E SO SE"};
static const char *opSymbols[] = {"^", "v", "->", "<->"};

static const Color BG       = {6, 10, 19, 255};
static const Color HEADER   = {11, 19, 32, 255};
static const Color PANEL    = {17, 29, 47, 255};
static const Color PANEL2   = {10, 18, 30, 255};
static const Color CARD     = {23, 38, 60, 255};
static const Color HOVER    = {33, 52, 77, 255};
static const Color SELECTED = {17, 62, 73, 255};
static const Color BORDER   = {49, 73, 101, 255};
static const Color TEXT     = {237, 244, 251, 255};
static const Color MUTED    = {145, 168, 193, 255};
static const Color CYAN     = {64, 220, 230, 255};
static const Color UI_GREEN  = {92, 228, 151, 255};
static const Color UI_RED    = {255, 91, 105, 255};
static const Color UI_GOLD   = {255, 200, 96, 255};
static const Color UI_PURPLE = {181, 137, 255, 255};

static Formula formulas[MAX_F];
static int count, stage, aSel = -1, bSel = -1, opSel = -1;
static int scroll, fusions, discoveries, errors;

static float pressure, soundTimer, flash;
static bool tracker, hintOn, solved, over, won, intro = true;

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
        bool v = false;

        switch (op) {
            case AND: v = p && q; break;
            case OR:  v = p || q; break;
            case IF:  v = !p || q; break;
            case IFF: v = p == q; break;
        }

        if (v) result |= (unsigned char)(1u << i);
    }

    return result;
}

static unsigned char NotTable(unsigned char table)
{
    return (unsigned char)(~table) & 0x0F;
}

static int Distance(unsigned char x, unsigned char y)
{
    unsigned char bits = x ^ y;
    int n = 0;

    for (int i = 0; i < 4; i++) n += (bits >> i) & 1;
    return n;
}

static bool Exists(const char *text)
{
    for (int i = 0; i < count; i++)
        if (strcmp(formulas[i].text, text) == 0) return true;

    return false;
}

static bool AddFormula(const char *text, unsigned char table)
{
    if (count >= MAX_F || Exists(text)) return false;

    snprintf(formulas[count].text, LEN, "%s", text);
    formulas[count++].table = table;
    discoveries++;
    return true;
}

static void AddStart(const char *text, unsigned char table)
{
    snprintf(formulas[count].text, LEN, "%s", text);
    formulas[count++].table = table;
}

static void Reset(void)
{
    count = 0;
    stage = 0;
    aSel = bSel = opSel = -1;
    scroll = fusions = discoveries = errors = 0;
    pressure = soundTimer = flash = 0;
    tracker = hintOn = solved = over = won = false;
    intro = true;

    AddStart("p", 0x0C);
    AddStart("q", 0x0A);
    AddStart("~p", 0x03);
    AddStart("~q", 0x05);

    Status("BIT ESTA PRONTO",
           "O rastreador comeca na primeira formula criada.",
           CYAN);
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

static void CheckAlarm(void)
{
    if (pressure >= 100.0f) {
        pressure = 100.0f;
        over = true;
        Status("ALARME DISPARADO",
               "O rastreador encontrou BIT. Reinicie a operacao.",
               RED);
    }
}

static void RegisterCreation(unsigned char result,
                             unsigned char sourceA,
                             unsigned char sourceB)
{
    tracker = true;

    int dResult = Distance(result, stages[stage].target);
    int dA = Distance(sourceA, stages[stage].target);
    int dB = Distance(sourceB, stages[stage].target);

    if (dResult > (dA < dB ? dA : dB))
        pressure += 16.0f;
    else
        pressure += 4.0f;

    CheckAlarm();
}

static void Solve(void)
{
    solved = true;
    flash = 1.0f;

    pressure -= 25.0f;
    if (pressure < 0.0f) pressure = 0.0f;

    Status(stages[stage].success, stages[stage].lesson, GREEN);
}

static void Failure(const char *message)
{
    errors++;

    if (tracker) pressure += 12.0f;
    CheckAlarm();

    if (errors >= 3 && !hintOn) {
        hintOn = true;
        Status("BIT: TENHO UMA PISTA", stages[stage].hint, GOLD);
    } else {
        Status("TENTATIVA NAO FUNCIONOU", message, RED);
    }
}

static void Fuse(void)
{
    if (over || won || solved) return;

    if (aSel < 0 || bSel < 0 || opSel < 0) {
        Status("FUSAO INCOMPLETA",
               "Selecione duas formulas e um conectivo.",
               GOLD);
        return;
    }

    Formula a = formulas[aSel];
    Formula b = formulas[bSel];

    char result[LEN];
    snprintf(result, sizeof(result), "(%s %s %s)",
             a.text, opSymbols[opSel], b.text);

    if (Exists(result)) {
        Failure("Essa fusao ja esta no caderno.");
        return;
    }

    unsigned char table = Eval(a.table, b.table, (Op)opSel);

    if (!AddFormula(result, table)) {
        Status("CADERNO CHEIO", "Nao ha espaco para novas formulas.", RED);
        return;
    }

    fusions++;
    RegisterCreation(table, a.table, b.table);
    if (over) return;

    errors = 0;
    hintOn = false;

    if (table == stages[stage].target) {
        Solve();
    } else if (table == 0x0F) {
        Status("TAUTOLOGIA DESCOBERTA",
               "A formula e verdadeira em todas as linhas.", GREEN);
    } else if (table == 0x00) {
        Status("CONTRADICAO DESCOBERTA",
               "A formula e falsa em todas as linhas.", GOLD);
    } else {
        Status("NOVA FORMULA",
               "Confira a tabela-verdade e tente combina-la.", CYAN);
    }
}

static void Negate(void)
{
    if (over || won || solved) return;

    int index = bSel >= 0 ? bSel : aSel;

    if (index < 0) {
        Status("SELECIONE UMA FORMULA",
               "Clique em uma formula e pressione N.",
               GOLD);
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
        Status("CADERNO CHEIO", "Nao ha espaco para novas formulas.", RED);
        return;
    }

    fusions++;
    RegisterCreation(table, source.table, source.table);
    if (over) return;

    errors = 0;
    hintOn = false;

    if (table == stages[stage].target) {
        Solve();
    } else {
        Status("NEGACAO DESCOBERTA",
               "Negar inverte verdadeiro e falso em cada linha.",
               CYAN);
    }
}

static void NextStage(void)
{
    if (!solved) return;

    if (stage == STAGES - 1) {
        won = true;
        return;
    }

    stage++;
    solved = false;
    aSel = bSel = opSel = -1;
    scroll = errors = 0;
    hintOn = false;

    Status("NOVA TRAVA LOCALIZADA",
           "A invasao continua. Use as formulas descobertas.",
           CYAN);
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
    DrawText(text, x - MeasureText(text, size) / 2, y, size, color);
}

static void DrawHeader(void)
{
    DrawRectangle(0, 0, W, 78, HEADER);
    DrawRectangle(0, 0, 6, 78, CYAN);

    DrawText("LOGIC CORE", 27, 10, 10, CYAN);
    DrawText("A FUGA DE BIT", 27, 27, 22, TEXT);
    DrawText("INVASAO LOGICA // ACESSO RESTRITO", 28, 54, 9, MUTED);

    DrawText(tracker ? "RASTREADOR ATIVO" : "RASTREADOR INATIVO",
             812, 15, 9, tracker ? GOLD : MUTED);

    Rectangle bar = {812, 34, 232, 15};
    DrawRectangleRounded(bar, 0.5f, 8, PANEL2);

    Color c = pressure < 55 ? CYAN : pressure < 80 ? GOLD : RED;
    if (pressure > 0) {
        DrawRectangleRounded(
            (Rectangle){812, 34, 232 * pressure / 100.0f, 15},
            0.5f, 8, c);
    }

    DrawText(TextFormat("%d%%", (int)pressure), 1052, 35, 10, TEXT);
    DrawLine(0, 77, W, 77, BORDER);
}

static void DrawTarget(void)
{
    Panel((Rectangle){28, 94, 1044, 129}, PANEL);

    DrawText(TextFormat("ALVO %02d / %02d", stage + 1, STAGES),
             48, 106, 9, CYAN);
    DrawText(stages[stage].name, 48, 125, 19, GOLD);
    DrawText(stages[stage].goal, 48, 152, 11, TEXT);

    DrawText(hintOn ? TextFormat("BIT: %s", stages[stage].hint)
                    : "BIT libera uma pista depois de 3 erros.",
             48, 179, 9, hintOn ? GOLD : MUTED);

    DrawCircle(897, 157, 38, PANEL2);
    DrawCircleLines(897, 157, 39, BORDER);

    Color lock = solved ? GREEN : pressure > 75 ? RED : CYAN;
    DrawRectangleRounded((Rectangle){877, 152, 40, 29}, 0.12f, 5, lock);
    DrawCircleLines(897, 149, 13, lock);
    DrawCircle(897, 160, 3, BG);
    DrawText(solved ? "ABERTO" : "TRAVA ATIVA",
             866, 205, 8, solved ? GREEN : MUTED);

    for (int i = 0; i < STAGES; i++) {
        Color dot = i < stage ? GREEN : i == stage ? GOLD : BORDER;
        DrawCircle(962 + (i % 3) * 25, 112 + (i / 3) * 28, 5, dot);
    }
}

static void DrawInventory(void)
{
    Panel((Rectangle){28, 239, 397, 474}, PANEL);
    DrawText("CADERNO DE BIT", 47, 255, 14, TEXT);
    DrawText("Clique para selecionar formulas.", 47, 277, 9, MUTED);
    DrawLine(47, 295, 406, 295, BORDER);

    for (int i = 0; i < VISIBLE && i + scroll < count; i++) {
        int index = i + scroll;
        Rectangle r = {
            47 + (i % 3) * 119,
            307 + (i / 3) * 75,
            111,
            67
        };

        bool selected = index == aSel || index == bSel;
        DrawRectangleRounded(r, 0.08f, 5,
                             selected ? SELECTED : Hover(r) ? HOVER : CARD);
        DrawRectangleRoundedLinesEx(r, 0.08f, 5, 1,
                                    selected ? CYAN : BORDER);

        DrawText(TextFormat("%02d", index + 1),
                 (int)r.x + 7, (int)r.y + 6, 8, MUTED);

        int size = 13;
        while (MeasureText(formulas[index].text, size) > r.width - 14 &&
               size > 8) size--;

        DrawText(formulas[index].text,
                 (int)r.x + 7, (int)r.y + 24, size, TEXT);

        Color tag = formulas[index].table == 0x0F ? GREEN :
                    formulas[index].table == 0x00 ? RED : MUTED;

        DrawText(formulas[index].table == 0x0F ? "SEMPRE V" :
                 formulas[index].table == 0x00 ? "SEMPRE F" : "VARIA",
                 (int)r.x + 7, (int)r.y + 46, 8, tag);

        if (Clicked(r)) SelectFormula(index);
    }

    if (count > VISIBLE)
        DrawText("Use a roda do mouse para ver mais.", 47, 610, 8, MUTED);

    DrawText(TextFormat("%d FORMULAS", count), 47, 677, 9, CYAN);
    DrawText(TextFormat("FUSOES: %d  DESCOBERTAS: %d", fusions, discoveries),
             47, 695, 8, MUTED);
}

static void DrawControls(void)
{
    Panel((Rectangle){443, 239, 629, 242}, PANEL);
    DrawText("MESA DE FUSAO", 466, 255, 15, TEXT);
    DrawText("Combine duas formulas com um conectivo.", 466, 277, 9, MUTED);
    DrawLine(466, 296, 1049, 296, BORDER);

    Panel((Rectangle){466, 307, 220, 43}, PANEL2);
    Panel((Rectangle){829, 307, 220, 43}, PANEL2);

    DrawText(aSel >= 0 ? formulas[aSel].text : "FORMULA A",
             477, 321, 12, aSel >= 0 ? TEXT : MUTED);
    DrawText(bSel >= 0 ? formulas[bSel].text : "FORMULA B",
             840, 321, 12, bSel >= 0 ? TEXT : MUTED);
    Center("+", 757, 315, 23, PURPLE);

    for (int i = 0; i < 4; i++) {
        Rectangle r = {466 + i * 147, 362, 140, 34};
        bool selected = opSel == i;

        DrawRectangleRounded(r, 0.08f, 5,
                             selected ? (Color){51, 37, 77, 255} :
                             Hover(r) ? HOVER : CARD);
        DrawRectangleRoundedLinesEx(r, 0.08f, 5, 1,
                                     selected ? PURPLE : BORDER);
        Center(opNames[i], (int)(r.x + r.width / 2),
               (int)r.y + 11, 8, selected ? PURPLE : TEXT);

        if (Clicked(r)) opSel = i;
    }

    Rectangle fuse = {466, 410, 367, 39};
    DrawRectangleRounded(fuse, 0.08f, 5, SELECTED);
    DrawRectangleRoundedLinesEx(fuse, 0.08f, 5, 1.5f, CYAN);
    Center("EXECUTAR FUSAO", 649, 422, 11, CYAN);
    if (Clicked(fuse)) Fuse();

    Rectangle negate = {846, 410, 203, 39};
    DrawRectangleRounded(negate, 0.08f, 5, CARD);
    DrawRectangleRoundedLinesEx(negate, 0.08f, 5, 1, BORDER);
    Center("NEGAR FORMULA [N]", 947, 423, 8, PURPLE);
    if (Clicked(negate)) Negate();

    DrawText("Criar formulas aumenta a pressao do rastreador.",
             466, 459, 8, MUTED);
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

    const char *rows[] = {"p=F q=F", "p=F q=V", "p=V q=F", "p=V q=V"};

    for (int i = 0; i < 4; i++) {
        bool value = (f.table >> i) & 1;
        int x = 466 + i * 143;

        DrawRectangleRounded((Rectangle){x, 610, 132, 40},
                             0.08f, 4, PANEL2);
        DrawText(rows[i], x + 7, 615, 8, MUTED);
        DrawText(value ? "VERDADEIRO" : "FALSO",
                 x + 7, 630, 8, value ? GREEN : RED);
    }

    DrawText(f.table == 0x0F ? "TAUTOLOGIA: sempre verdadeira." :
             f.table == 0x00 ? "CONTRADICAO: sempre falsa." :
             "CONTINGENTE: depende dos valores.",
             466, 670, 8,
             f.table == 0x0F ? GREEN : f.table == 0x00 ? RED : GOLD);
}

static void Overlay(const char *title, const char *mainText,
                    const char *detail, const char *button, Color accent)
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
        if (over || won) Reset();
        else NextStage();
    }
}

static void DrawIntro(void)
{
    DrawRectangle(0, 0, W, H, (Color){2, 5, 12, 220});
    Panel((Rectangle){250, 225, 600, 300}, PANEL);

    Center("MISSAO: FUGA DE BIT", W / 2, 255, 20, CYAN);
    Center("Combine formulas para superar as travas.", W / 2, 300, 12, TEXT);
    Center("Selecione duas formulas, escolha um conectivo e funda.", W / 2, 330, 10, MUTED);
    Center("Pressione N para negar uma formula.", W / 2, 352, 10, MUTED);
    Center("A pressao sobe com o tempo; cada trava superada reduz 25%.",
           W / 2, 374, 10, GREEN);

    Rectangle r = {400, 420, 300, 46};
    DrawRectangleRounded(r, 0.1f, 6, SELECTED);
    DrawRectangleRoundedLinesEx(r, 0.1f, 6, 2, CYAN);
    Center("COMECAR", W / 2, 435, 12, TEXT);

    if (Clicked(r)) intro = false;
}

int main(void)
{
    InitWindow(W, H, "LOGIC CORE | A Fuga de BIT");
    SetTargetFPS(60);
    Reset();

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        if (!intro && IsKeyPressed(KEY_R)) Reset();

        if (!intro && !over && !won && !solved) {
            if (IsKeyPressed(KEY_N)) Negate();
            if (IsKeyPressed(KEY_ENTER)) Fuse();

            if (tracker) {
                pressure += (1.5f + pressure * 0.02f) * dt;
                CheckAlarm();

                soundTimer -= dt;
                if (soundTimer <= 0.0f) {
                    soundTimer = 1.25f - pressure * 0.0085f;
                    if (soundTimer < 0.35f) soundTimer = 0.35f;
                }
            }
        }

        Rectangle inventory = {28, 239, 397, 474};
        if (Hover(inventory)) {
            scroll -= (int)GetMouseWheelMove();
            int maxScroll = count - VISIBLE;
            if (maxScroll < 0) maxScroll = 0;
            if (scroll < 0) scroll = 0;
            if (scroll > maxScroll) scroll = maxScroll;
        } else {
            GetMouseWheelMove();
        }

        if (flash > 0.0f) {
            flash -= dt;
            if (flash < 0.0f) flash = 0.0f;
        }

        BeginDrawing();
        ClearBackground(BG);

        DrawHeader();
        DrawTarget();
        DrawInventory();
        DrawControls();
        DrawAnalysis();

        DrawText("N: negar   ENTER: fundir   R: reiniciar",
                 28, 735, 9, MUTED);

        if (flash > 0.0f) {
            DrawRectangle(0, 78, W, H - 78,
                          (Color){91, 228, 151, (unsigned char)(flash * 55)});
        }

        if (solved && !won) {
            Overlay("TRAVA SUPERADA",
                    stages[stage].success,
                    stages[stage].lesson,
                    stage + 1 < STAGES ? "PROXIMO DESAFIO" : "ABRIR O COFRE",
                    GREEN);
        }

        if (over) {
            Overlay("ALARME DISPARADO",
                    "O rastreador encontrou BIT.",
                    "Reinicie a operacao e tente outra sequencia.",
                    "REINICIAR", RED);
        }

        if (won) {
            Overlay("OPERACAO CONCLUIDA",
                    "BIT escapou com os dados.",
                    TextFormat("%d fusoes e %d formulas descobertas.",
                               fusions, discoveries),
                    "JOGAR NOVAMENTE", GOLD);
        }

        if (intro) DrawIntro();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}