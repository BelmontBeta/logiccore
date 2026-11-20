
#include "game.h"

#include "../systems/player.h"
#include "../scenes/room.h"

#include "../minigames/cipher/cipher.h"
#include "../minigames/doodle/doodle.h"
#include "../minigames/terminal/terminal.h"

typedef enum {
    MODE_EXPLORATION,
    MODE_CIPHER,
    MODE_DOODLE,
    MODE_TERMINAL
} GameMode;

#define TOTAL_ROOMS 3

static GameState game;
static GameMode mode;

static Rectangle vault = {900, 350, 100, 220};

static bool showMessage;
static bool vaultUnlocked;

static void StartCurrentMinigame(void)
{
    switch (game.currentRoom) {
        case 1:
            mode = MODE_CIPHER;
            Cipher_Init(&game.lives);
            break;

        case 2:
            mode = MODE_DOODLE;
            Doodle_Init(&game.lives);
            break;

        case 3:
            mode = MODE_TERMINAL;
            Terminal_Init(&game.lives);
            break;

        default:
            game.gameRunning = false;
            break;
    }
}

static void UnloadCurrentMinigame(void)
{
    switch (mode) {
        case MODE_CIPHER:
            Cipher_Unload();
            break;

        case MODE_DOODLE:
            Doodle_Unload();
            break;

        case MODE_TERMINAL:
            Terminal_Unload();
            break;

        default:
            break;
    }
}

static void CompleteRoom(void)
{
    UnloadCurrentMinigame();

    game.currentRoom++;

    if (game.currentRoom > TOTAL_ROOMS) {
        game.gameRunning = false;
        return;
    }

    mode = MODE_EXPLORATION;
    vaultUnlocked = false;
    showMessage = false;

    game.player.position.x = 100;
}

void Game_Init(void)
{
    game.lives = 5;
    game.currentRoom = 1;
    game.gameRunning = true;

    mode = MODE_EXPLORATION;
    showMessage = false;
    vaultUnlocked = false;

    Player_Init(&game.player, (Vector2){100, 520});
}

void Game_Update(float dt)
{
    if (!game.gameRunning)
        return;

    if (game.lives <= 0) {
        game.lives = 0;
        game.gameRunning = false;
        return;
    }

    if (mode != MODE_EXPLORATION) {
        bool completed = false;
        bool gameOver = false;

        switch (mode) {
            case MODE_CIPHER: {
                CipherResult r = Cipher_Update();
                completed = r == CIPHER_COMPLETED;
                gameOver = r == CIPHER_GAME_OVER;
                break;
            }

            case MODE_DOODLE: {
                DoodleResult r = Doodle_Update(dt);
                completed = r == DOODLE_COMPLETED;
                gameOver = r == DOODLE_GAME_OVER;
                break;
            }

            case MODE_TERMINAL: {
                TerminalResult r = Terminal_Update(dt);
                completed = r == TERMINAL_COMPLETED;
                gameOver = r == TERMINAL_GAME_OVER;
                break;
            }

            default:
                break;
        }

        if (completed)
            CompleteRoom();
        else if (gameOver || game.lives <= 0) {
            UnloadCurrentMinigame();
            game.lives = 0;
            game.gameRunning = false;
        }

        return;
    }

    Player_Update(
        &game.player,
        dt,
        Room_GetBounds()
    );

    Rectangle playerBounds = {
        game.player.position.x,
        game.player.position.y,
        game.player.size.x,
        game.player.size.y
    };

    Rectangle interactionArea = {
        vault.x - 35,
        vault.y + vault.height - 70,
        vault.width + 70,
        70
    };

    showMessage = CheckCollisionRecs(
        playerBounds,
        interactionArea
    );

    if (showMessage &&
        IsKeyPressed(KEY_E) &&
        !vaultUnlocked) {

        StartCurrentMinigame();
    }
}

void Game_Draw(void)
{
    if (mode == MODE_CIPHER) {
        Room_Draw();
        Cipher_Draw();
        return;
    }

    if (mode == MODE_DOODLE) {
        Room_Draw();
        Doodle_Draw();
        return;
    }

    if (mode == MODE_TERMINAL) {
        Room_Draw();
        Terminal_Draw();
        return;
    }

    Room_Draw();

    Color vaultColor = vaultUnlocked ? GREEN : SKYBLUE;

    DrawRectangleRec(vault, vaultColor);
    DrawRectangleLinesEx(vault, 3, WHITE);

    DrawText(
        vaultUnlocked ? "ABERTO" : "COFRE",
        915, 450, 20, WHITE
    );

    Player_Draw(&game.player);

    DrawText("LOGIC CORE", 50, 25, 25, WHITE);

    DrawText(
        TextFormat("SALA %02d / %02d", game.currentRoom, TOTAL_ROOMS),
        850, 25, 20, LIGHTGRAY
    );

    DrawText(
        TextFormat("VIDAS: %d", game.lives),
        50, 650, 20, WHITE
    );

    if (showMessage && !vaultUnlocked) {
        DrawRectangle(300, 680, 500, 45, DARKBLUE);

        DrawText(
            "PRESSIONE E PARA INTERAGIR",
            345, 692, 20, WHITE
        );
    }

    if (!game.gameRunning) {
        DrawRectangle(250, 300, 600, 120, DARKBLUE);

        DrawText(
            game.lives <= 0 ? "FIM DE JOGO" : "CAMPANHA CONCLUIDA!",
            350, 345, 30, WHITE
        );
    }
}

void Game_Unload(void)
{
    UnloadCurrentMinigame();
}
