
#include "cipher.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define MAX_INPUT 256
#define TOKEN_COUNT 9
#define LEVEL_COUNT 5

typedef struct {
    const char *difficulty;
    const char *description;
    const char *variables;
    const char *expected;
} Level;

typedef struct {
    const char *text;
    int position;
    bool p, q, r;
    bool valid;
} Parser;

static const char *tokens[TOKEN_COUNT] = {
    "(", ")", "p", "q", "r", " v ", " ^ ", " -> ", "¬"
};

static const char *labels[TOKEN_COUNT] = {
    "(", ")", "p", "q", "r", "v", "^", "->", "¬"
};

static const Level levels[LEVEL_COUNT] = {
    {"FACIL",
     "O sensor detecta movimento ou a luz apaga.",
     "p = sensor detecta movimento    q = luz apaga",
     "p v q"},

    {"FACIL",
     "Se o cartao for autorizado, a porta sera destrancada.",
     "p = cartao autorizado    q = porta destrancada",
     "p -> q"},

    {"MEDIA",
     "Se o reator superaquecer e a refrigeracao falhar, o protocolo de emergencia sera ativado.",
     "p = reator superaquece    q = refrigeracao falha    r = emergencia ativa",
     "(p ^ q) -> r"},

    {"MEDIA",
     "Se a nave receber o sinal, a equipe inicia a aterrissagem ou ativa o piloto automatico.",
     "p = nave recebe sinal    q = inicia aterrissagem    r = piloto automatico",
     "p -> (q v r)"},

    {"DIFICIL",
     "Se o sensor nao detectar movimento ou a camera gravar, o alarme sera ativado.",
     "p = sensor detecta movimento    q = camera grava    r = alarme ativa",
     "(¬p v q) -> r"},

};

static int currentLevel;
static char input[MAX_INPUT];
static char message[160];
static int *campaignLives;
static CipherResult result;

static Rectangle tokenButtons[TOKEN_COUNT];

static bool ButtonClicked(Rectangle bounds)
{
    return CheckCollisionPointRec(GetMousePosition(), bounds)
        && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

static void DrawButton(Rectangle bounds, const char *label)
{
    bool hover = CheckCollisionPointRec(GetMousePosition(), bounds);

    DrawRectangleRec(
        bounds,
        hover ? (Color){57, 68, 89, 255}
              : (Color){36, 43, 59, 255}
    );

    DrawRectangleLinesEx(bounds, 2, SKYBLUE);

    int width = MeasureText(label, 20);

    DrawText(
        label,
        (int)(bounds.x + (bounds.width - width) / 2),
        (int)(bounds.y + (bounds.height - 20) / 2),
        20,
        WHITE
    );
}

static void AddToken(const char *token)
{
    size_t length = strlen(input);

    if (length + strlen(token) < MAX_INPUT)
        strcat(input, token);
}

static void RemoveLastToken(void)
{
    size_t length = strlen(input);

    if (length == 0)
        return;

    while (length > 0 && input[length - 1] == ' ')
        input[--length] = '\0';

    for (int i = 0; i < TOKEN_COUNT; i++) {
        size_t tokenLength = strlen(tokens[i]);

        if (length >= tokenLength &&
            strncmp(input + length - tokenLength,
                    tokens[i], tokenLength) == 0) {

            input[length - tokenLength] = '\0';

            length = strlen(input);

            while (length > 0 && input[length - 1] == ' ')
                input[--length] = '\0';

            return;
        }
    }

    input[length - 1] = '\0';
}

static void SkipSpaces(Parser *parser)
{
    while (parser->text[parser->position] == ' ')
        parser->position++;
}

static bool Match(Parser *parser, const char *text)
{
    SkipSpaces(parser);

    size_t length = strlen(text);

    if (strncmp(parser->text + parser->position, text, length) == 0) {
        parser->position += (int)length;
        return true;
    }

    return false;
}

static bool ParseImplication(Parser *parser);

static bool ParsePrimary(Parser *parser)
{
    SkipSpaces(parser);

    if (Match(parser, "(")) {
        bool value = ParseImplication(parser);

        if (!Match(parser, ")"))
            parser->valid = false;

        return value;
    }

    char current = parser->text[parser->position];

    if (current == 'p') {
        parser->position++;
        return parser->p;
    }

    if (current == 'q') {
        parser->position++;
        return parser->q;
    }

    if (current == 'r') {
        parser->position++;
        return parser->r;
    }

    parser->valid = false;
    return false;
}

static bool ParseNot(Parser *parser)
{
    SkipSpaces(parser);

    if (strncmp(parser->text + parser->position, "¬", strlen("¬")) == 0) {
        parser->position += (int)strlen("¬");
        return !ParseNot(parser);
    }

    if (parser->text[parser->position] == '~') {
        parser->position++;
        return !ParseNot(parser);
    }

    return ParsePrimary(parser);
}

static bool ParseAnd(Parser *parser)
{
    bool value = ParseNot(parser);

    while (parser->valid) {
        SkipSpaces(parser);

        if (parser->text[parser->position] != '^')
            break;

        parser->position++;
        bool right = ParseNot(parser);
        value = value && right;
    }

    return value;
}

static bool ParseOr(Parser *parser)
{
    bool value = ParseAnd(parser);

    while (parser->valid) {
        SkipSpaces(parser);

        if (parser->text[parser->position] != 'v')
            break;

        parser->position++;
        bool right = ParseAnd(parser);
        value = value || right;
    }

    return value;
}

static bool ParseImplication(Parser *parser)
{
    bool left = ParseOr(parser);

    if (!parser->valid)
        return false;

    SkipSpaces(parser);

    if (strncmp(parser->text + parser->position, "->", 2) == 0) {
        parser->position += 2;
        bool right = ParseImplication(parser);
        return (!left) || right;
    }

    return left;
}

static bool EvaluateFormula(
    const char *text,
    bool p,
    bool q,
    bool r,
    bool *answer
)
{
    Parser parser = {text, 0, p, q, r, true};

    bool value = ParseImplication(&parser);

    SkipSpaces(&parser);

    if (!parser.valid || parser.text[parser.position] != '\0')
        return false;

    *answer = value;
    return true;
}

static bool EquivalentFormulas(const char *answer, const char *expected)
{
    for (int row = 0; row < 8; row++) {
        bool p = (row & 4) != 0;
        bool q = (row & 2) != 0;
        bool r = (row & 1) != 0;

        bool answerValue;
        bool expectedValue;

        if (!EvaluateFormula(answer, p, q, r, &answerValue))
            return false;

        if (!EvaluateFormula(expected, p, q, r, &expectedValue))
            return false;

        if (answerValue != expectedValue)
            return false;
    }

    return true;
}

void Cipher_Init(int *lives)
{
    campaignLives = lives;
    currentLevel = 0;
    input[0] = '\0';
    strcpy(message, "Monte a formula usando os botoes.");
    result = CIPHER_RUNNING;
}

CipherResult Cipher_Update(void)
{
    if (result != CIPHER_RUNNING)
        return result;

    for (int i = 0; i < TOKEN_COUNT; i++) {
        int row = i / 5;
        int column = i % 5;

        tokenButtons[i] = (Rectangle){
            105 + column * 100,
            row == 0 ? 475 : 530,
            90,
            45
        };

        if (ButtonClicked(tokenButtons[i]))
            AddToken(tokens[i]);
    }

    if (ButtonClicked((Rectangle){650, 475, 130, 45}))
        RemoveLastToken();

    if (ButtonClicked((Rectangle){650, 530, 130, 45}))
        input[0] = '\0';

    if (ButtonClicked((Rectangle){800, 475, 130, 100})) {

        bool correct;

        if (currentLevel == LEVEL_COUNT - 1) {
            correct = strcmp(input, levels[currentLevel].expected) == 0;
        } else {
            correct = EquivalentFormulas(
                input,
                levels[currentLevel].expected
            );
        }

        if (correct) {
            currentLevel++;
            input[0] = '\0';

            if (currentLevel >= LEVEL_COUNT) {
                result = CIPHER_COMPLETED;
            } else {
                strcpy(message, "Correto! Proxima fase.");
            }
        } else {
            (*campaignLives)--;

            input[0] = '\0';

            if (*campaignLives <= 0) {
                result = CIPHER_GAME_OVER;
            } else {
                strcpy(message, "Formula incorreta! Voce perdeu uma vida.");
            }
        }
    }

    return result;
}

void Cipher_Draw(void)
{
    DrawRectangle(70, 110, 960, 600, (Color){17, 21, 32, 255});

    DrawText("CIPHER LOGIC", 100, 125, 32, SKYBLUE);
    DrawText("INTERCEPTACAO DE RADIO", 102, 165, 18, GOLD);

    if (result == CIPHER_COMPLETED) {
        DrawText("TODAS AS FASES CONCLUIDAS!", 230, 350, 30, GREEN);
        return;
    }

    if (result == CIPHER_GAME_OVER) {
        DrawText("FIM DE JOGO", 400, 350, 35, RED);
        return;
    }

    const Level *level = &levels[currentLevel];

    DrawText(
        TextFormat("FASE %d / %d", currentLevel + 1, LEVEL_COUNT),
        820, 130, 20, WHITE
    );

    DrawText(
        TextFormat("VIDAS: %d", *campaignLives),
        820, 160, 20, GREEN
    );

    DrawText(level->difficulty, 105, 220, 20, GOLD);

    DrawText("TRADUZA A FRASE:", 105, 260, 18, GOLD);
    DrawText(level->description, 105, 295, 18, WHITE);

    DrawText(level->variables, 105, 355, 15, LIGHTGRAY);

    DrawText("SUA FORMULA:", 105, 410, 18, GOLD);

    DrawRectangle(100, 435, 830, 35, (Color){35, 43, 59, 255});
    DrawText(input[0] ? input : "_", 110, 442, 20, WHITE);

    for (int i = 0; i < TOKEN_COUNT; i++)
        DrawButton(tokenButtons[i], labels[i]);

    DrawButton((Rectangle){650, 475, 130, 45}, "Apagar");
    DrawButton((Rectangle){650, 530, 130, 45}, "Limpar");
    DrawButton((Rectangle){800, 475, 130, 100}, "Verificar");

    DrawText(message, 105, 620, 18, LIGHTGRAY);
}

void Cipher_Unload(void)
{
    campaignLives = NULL;
}
