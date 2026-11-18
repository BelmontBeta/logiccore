#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define MAX_INPUT 256
#define MAX_LIVES 5
#define TOKEN_COUNT 9

typedef struct {
    const char *difficulty;
    const char *description;
    const char *variables;
    const char *expected;
} Level;

typedef struct {
    const char *text;
    int position;
    bool p;
    bool q;
    bool r;
    bool valid;
} Parser;

static const char *tokens[TOKEN_COUNT] = {
    "(", ")", "p", "q", "r", " v ", " ^ ", " -> ", "¬"
};

static const char *labels[TOKEN_COUNT] = {
    "(", ")", "p", "q", "r", "v", "^", "->", "¬"
};

static const Level levels[] = {
    {
        "FACIL",
        "O sensor detecta movimento ou a luz apaga.",
        "p = sensor detecta movimento    q = luz apaga",
        "p v q"
    },
    {
        "FACIL",
        "Se o cartao for autorizado, a porta sera destrancada.",
        "p = cartao autorizado    q = porta destrancada",
        "p -> q"
    },
    {
        "MEDIA",
        "Se o reator superaquecer e a refrigeracao falhar, o protocolo de emergencia sera ativado.",
        "p = reator superaquece    q = refrigeracao falha    r = emergencia ativa",
        "(p ^ q) -> r"
    },
    {
        "MEDIA",
        "Se a nave receber o sinal, a equipe inicia a aterrissagem ou ativa o piloto automatico.",
        "p = nave recebe sinal    q = inicia aterrissagem    r = piloto automatico",
        "p -> (q v r)"
    },
    {
        "DIFICIL",
        "Se o sensor nao detectar movimento ou a camera gravar, o alarme sera ativado.",
        "p = sensor detecta movimento    q = camera grava    r = alarme ativa",
        "(¬p v q) -> r"
    },
    {
        "DIFICIL",
        "O alerta sera ativado se o reator superaquecer e a refrigeracao falhar nao ocorrerem juntos.",
        "p = reator superaquece    q = refrigeracao falha    r = alerta ativa",
        "¬(p ^ q) v r"
    }
};

#define LEVEL_COUNT ((int)(sizeof(levels) / sizeof(levels[0])))

static bool Button(Rectangle bounds, const char *text)
{
    Vector2 mouse = GetMousePosition();
    bool hover = CheckCollisionPointRec(mouse, bounds);

    DrawRectangleRec(
        bounds,
        hover ? (Color){57, 68, 89, 255} : (Color){36, 43, 59, 255}
    );
    DrawRectangleLinesEx(bounds, 2, SKYBLUE);

    int textWidth = MeasureText(text, 20);

    DrawText(
        text,
        (int)(bounds.x + (bounds.width - textWidth) / 2),
        (int)bounds.y + (int)(bounds.height - 20) / 2,
        20,
        RAYWHITE
    );

    return hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

static void AddText(char *input, const char *text)
{
    size_t inputLength = strlen(input);
    size_t textLength = strlen(text);

    if (inputLength + textLength < MAX_INPUT)
        strcat(input, text);
}

static void RemoveLastToken(char *input)
{
    size_t inputLength = strlen(input);

    if (inputLength == 0)
        return;

    while (inputLength > 0 && input[inputLength - 1] == ' ')
        input[--inputLength] = '\0';

    for (int i = 0; i < TOKEN_COUNT; i++) {
        size_t tokenLength = strlen(tokens[i]);

        if (inputLength >= tokenLength &&
            strncmp(input + inputLength - tokenLength,
                    tokens[i],
                    tokenLength) == 0) {
            input[inputLength - tokenLength] = '\0';

            inputLength = strlen(input);
            while (inputLength > 0 && input[inputLength - 1] == ' ')
                input[--inputLength] = '\0';

            return;
        }
    }

    input[inputLength - 1] = '\0';
}

/* Ignora espacos durante a leitura da formula. */
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

/*
    Gramatica e precedencia:
    parenteses e negacao
    E (^)
    OU (v)
    implicacao (->), associativa a direita
*/

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

    if (parser->text[parser->position] == 'p') {
        parser->position++;
        return parser->p;
    }

    if (parser->text[parser->position] == 'q') {
        parser->position++;
        return parser->q;
    }

    if (parser->text[parser->position] == 'r') {
        parser->position++;
        return parser->r;
    }

    parser->valid = false;
    return false;
}

static bool ParseNot(Parser *parser)
{
    SkipSpaces(parser);

    /* Aceita o simbolo logico ¬ e tambem ~ como negacao. */
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
    bool *result
)
{
    Parser parser = {
        text,
        0,
        p,
        q,
        r,
        true
    };

    bool value = ParseImplication(&parser);
    SkipSpaces(&parser);

    if (!parser.valid || parser.text[parser.position] != '\0')
        return false;

    *result = value;
    return true;
}

/*
    Compara as formulas nas oito combinacoes possiveis de p, q e r.
    Isso permite aceitar respostas logicamente equivalentes, mesmo
    quando a escrita nao e identica a formula esperada.
*/
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

static void DrawWrappedText(
    const char *text,
    Rectangle bounds,
    int fontSize,
    float spacing,
    Color color
)
{
    Font font = GetFontDefault();
    float lineHeight = (float)fontSize + 5.0f;
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
            (float)fontSize,
            spacing
        ).x;

        float spaceWidth = MeasureTextEx(
            font,
            " ",
            (float)fontSize,
            spacing
        ).x;

        if (x > bounds.x && x + wordWidth > bounds.x + bounds.width) {
            x = bounds.x;
            y += lineHeight;
        }

        DrawTextEx(
            font,
            wordBuffer,
            (Vector2){x, y},
            (float)fontSize,
            spacing,
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

static void ResetGame(
    int *levelIndex,
    int *lives,
    bool *gameOver,
    bool *gameWon,
    char *input,
    char *message
)
{
    *levelIndex = 0;
    *lives = MAX_LIVES;
    *gameOver = false;
    *gameWon = false;
    input[0] = '\0';

    strcpy(message, "Monte a formula usando os botoes.");
}

int main(void)
{
    InitWindow(900, 700, "Logic Core - Cipher Logic");
    SetTargetFPS(60);
    SetRandomSeed((unsigned int)time(NULL));

    char input[MAX_INPUT] = "";
    char message[160] = "Monte a formula usando os botoes.";

    int levelIndex = 0;
    int lives = MAX_LIVES;

    bool gameOver = false;
    bool gameWon = false;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground((Color){17, 21, 32, 255});

        DrawText("CIPHER LOGIC", 40, 25, 34, SKYBLUE);
        DrawText("INTERCEPTACAO DE RADIO", 42, 72, 18, GOLD);

        if (!gameOver && !gameWon) {
            const Level *currentLevel = &levels[levelIndex];

            DrawText(
                TextFormat("FASE %d / %d", levelIndex + 1, LEVEL_COUNT),
                710,
                30,
                20,
                RAYWHITE
            );

            DrawText(
                TextFormat("VIDAS: %d", lives),
                710,
                62,
                20,
                lives <= 2 ? RED : GREEN
            );

            DrawText(currentLevel->difficulty, 710, 94, 18, GOLD);

            DrawRectangleRounded(
                (Rectangle){40, 125, 820, 145},
                0.1f,
                8,
                (Color){26, 33, 47, 255}
            );

            DrawText("TRADUZA A FRASE:", 58, 139, 16, GOLD);

            DrawWrappedText(
                currentLevel->description,
                (Rectangle){58, 166, 780, 70},
                18,
                1.0f,
                WHITE
            );

            DrawWrappedText(
                currentLevel->variables,
                (Rectangle){45, 278, 810, 42},
                16,
                1.0f,
                LIGHTGRAY
            );

            DrawText("MONTE A FORMULA", 45, 325, 18, GOLD);

            DrawRectangleRounded(
                (Rectangle){45, 353, 810, 58},
                0.12f,
                8,
                (Color){26, 33, 47, 255}
            );

            DrawText(input[0] ? input : "_", 62, 370, 22, WHITE);

            const float buttonWidth = 86.0f;
            const float buttonHeight = 46.0f;
            const float gap = 8.0f;
            const float startX = 45.0f;
            const float row1Y = 425.0f;
            const float row2Y = 479.0f;

            for (int i = 0; i < TOKEN_COUNT; i++) {
                int row = i / 5;
                int column = i % 5;

                Rectangle bounds = {
                    startX + column * (buttonWidth + gap),
                    row == 0 ? row1Y : row2Y,
                    buttonWidth,
                    buttonHeight
                };

                if (Button(bounds, labels[i]))
                    AddText(input, tokens[i]);
            }

            if (Button((Rectangle){525, 425, 145, 46}, "Apagar"))
                RemoveLastToken(input);

            if (Button((Rectangle){680, 425, 150, 46}, "Verificar")) {
                if (EquivalentFormulas(input, currentLevel->expected)) {
                    levelIndex++;
                    input[0] = '\0';

                    if (levelIndex >= LEVEL_COUNT) {
                        gameWon = true;
                    } else {
                        strcpy(message, "Correto! Proxima fase.");
                    }
                } else {
                    lives--;

                    if (lives <= 0) {
                        gameOver = true;
                    } else {
                        strcpy(
                            message,
                            "Incorreto. Voce perdeu uma vida."
                        );
                    }
                }
            }

            if (Button((Rectangle){525, 479, 145, 46}, "Limpar")) {
                input[0] = '\0';
                strcpy(message, "Monte a formula usando os botoes.");
            }

            DrawText(message, 45, 555, 18, LIGHTGRAY);
        } else if (gameWon) {
            DrawText("MISSAO CONCLUIDA!", 235, 230, 38, GREEN);
            DrawText(
                "Voce resolveu todas as fases.",
                260,
                295,
                22,
                RAYWHITE
            );

            if (Button((Rectangle){350, 380, 200, 55}, "Jogar novamente")) {
                ResetGame(
                    &levelIndex,
                    &lives,
                    &gameOver,
                    &gameWon,
                    input,
                    message
                );
            }
        } else {
            DrawText("FIM DE JOGO", 320, 230, 40, RED);
            DrawText("Voce ficou sem vidas.", 330, 295, 22, RAYWHITE);

            if (Button((Rectangle){350, 380, 200, 55}, "Tentar novamente")) {
                ResetGame(
                    &levelIndex,
                    &lives,
                    &gameOver,
                    &gameWon,
                    input,
                    message
                );
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}