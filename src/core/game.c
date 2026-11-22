
#include "game.h"

#include "../systems/player.h"
#include "../scenes/room.h"
#include "../scenes/menu.h"
#include "../scenes/memory.h"

#include "../minigames/cipher/cipher.h"
#include "../minigames/doodle/doodle.h"
#include "../minigames/terminal/terminal.h"

typedef enum {
    MODE_MENU,
    MODE_MEMORY,
    MODE_EXPLORATION,
    MODE_CIPHER,
    MODE_DOODLE,
    MODE_TERMINAL,
    MODE_GAME_OVER
} GameMode;

typedef enum {
    AFTER_INTRO,
    AFTER_ROOM1,
    AFTER_ROOM2,
    AFTER_DISCOVERY,
    AFTER_ENDING
} MemoryNextAction;

#define TOTAL_ROOMS 3

static GameState game;
static GameMode mode;
static Texture2D doorClosed;

static Rectangle vault = {900, 350, 100, 220};

static Rectangle energyPickup = {535, 500, 30, 42};
static Rectangle energyPickupRoom3 = {700, 600, 30, 42};

static bool energyCollected;
static bool showMessage;
static bool vaultUnlocked;
static bool campaignCompleted;

static MemoryNextAction memoryNextAction;

static void StartMemory(
    MemorySequence sequence,
    MemoryNextAction nextAction
)
{
    Memory_Start(sequence);
    memoryNextAction = nextAction;
    mode = MODE_MEMORY;
}

static void ContinueAfterMemory(void)
{
    switch (memoryNextAction) {

        case AFTER_INTRO:
            mode = MODE_EXPLORATION;
            break;

        case AFTER_ROOM1:
            mode = MODE_EXPLORATION;
            break;

        case AFTER_ROOM2:
            mode = MODE_EXPLORATION;
            break;

        case AFTER_DISCOVERY:
            StartMemory(
                MEMORY_ENDING,
                AFTER_ENDING
            );
            break;

        case AFTER_ENDING:
            campaignCompleted = true;
            game.currentRoom = 1;
            game.lives = 100;
            game.player.position = (Vector2){100, 442};

            vaultUnlocked = false;
            showMessage = false;
            energyCollected = false;

            mode = MODE_MENU;
            break;
    }
}

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

        game.currentRoom = TOTAL_ROOMS;

        StartMemory(
            MEMORY_DISCOVERY,
            AFTER_DISCOVERY
        );

        return;
    }

    vaultUnlocked = false;
    showMessage = false;
    energyCollected = false;

    game.player.position.x = 100;

    if (game.currentRoom == 2) {

        game.player.position.y = 442;

        StartMemory(
            MEMORY_BETWEEN_ROOMS,
            AFTER_ROOM1
        );

        return;
    }

    if (game.currentRoom == 3) {

        // BIT mais próximo da passarela da Sala 3.
        game.player.position.y = 555;

        StartMemory(
            MEMORY_ROOM2,
            AFTER_ROOM2
        );

        return;
    }

    game.player.position.y = 442;
    mode = MODE_EXPLORATION;
}

static void RestartCampaign(void)
{
    game.lives = 100;
    game.currentRoom = 1;
    game.gameRunning = true;

    game.player.position = (Vector2){100, 442};
    game.player.currentFrame = 0;
    game.player.animationTimer = 0;
    game.player.moving = false;
    game.player.facingLeft = false;

    vaultUnlocked = false;
    showMessage = false;
    energyCollected = false;
    campaignCompleted = false;

    mode = MODE_EXPLORATION;
}

void Game_Init(void)
{
    game.lives = 100;
    game.currentRoom = 1;
    game.gameRunning = true;

    mode = MODE_MENU;

    showMessage = false;
    vaultUnlocked = false;
    energyCollected = false;
    campaignCompleted = false;

    Room_Init();
    Menu_Init();
    Memory_Init();

    doorClosed = LoadTexture(
        "assets/backgrounds/portafechada.png"
    );

    Player_Init(
        &game.player,
        (Vector2){100, 442}
    );
}

void Game_Update(float dt)
{
   if (mode == MODE_GAME_OVER) {
    Rectangle restartButton = {400, 450, 300, 65};

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        CheckCollisionPointRec(GetMousePosition(), restartButton)) {
        RestartCampaign();
    }

    return;
}

if (!game.gameRunning)
    return;

if (game.lives <= 0) {
    game.lives = 0;
    game.gameRunning = false;
    mode = MODE_GAME_OVER;
    return;
}

    // Menu inicial e reinicio da campanha.
    if (mode == MODE_MENU) {

        if (Menu_Update()) {

            game.currentRoom = 1;
            game.lives = 100;
            game.player.position = (Vector2){100, 442};

            vaultUnlocked = false;
            showMessage = false;
            energyCollected = false;
            campaignCompleted = false;

            StartMemory(
                MEMORY_INTRO,
                AFTER_INTRO
            );
        }

        return;
    }

    // Sequencias narrativas.
    if (mode == MODE_MEMORY) {

        if (Memory_Update(dt)) {
            ContinueAfterMemory();
        }

        return;
    }

    // Minigames.
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

        if (completed) {

            CompleteRoom();

        } else if (gameOver || game.lives <= 0) {
            UnloadCurrentMinigame();

            game.lives = 0;
            game.gameRunning = false;
            mode = MODE_GAME_OVER;
        }

        return;
    }

    // Movimentacao do BIT.
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

    // Coletaveis de energia.
    Rectangle currentEnergyPickup =
        game.currentRoom == 3
            ? energyPickupRoom3
            : energyPickup;

    if ((game.currentRoom == 2 || game.currentRoom == 3) &&
        !energyCollected &&
        CheckCollisionRecs(playerBounds, currentEnergyPickup)) {

        game.lives += 20;

        if (game.lives > 100)
            game.lives = 100;

        energyCollected = true;
    }

    // Interacao com o cofre ou computador.
    Rectangle interactionArea;

    if (game.currentRoom == 3) {

        // Estacao de computador no canto direito.
        interactionArea = (Rectangle){
            830, 500, 270, 200
        };

    } else {

        interactionArea = (Rectangle){
            vault.x - 35,
            vault.y + vault.height - 70,
            vault.width + 70,
            70
        };
    }

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

    //gameover
    if (mode == MODE_GAME_OVER) {
        DrawRectangle(0, 0, 1100, 760, (Color){5, 10, 22, 245});

        DrawText(
            "SISTEMA INTERROMPIDO",
            275, 250, 42, RED
        );

        DrawText(
            "A bateria de BIT foi esgotada.",
            350, 330, 22, LIGHTGRAY
        );

        Rectangle restartButton = {400, 450, 300, 65};

        bool hovering = CheckCollisionPointRec(
            GetMousePosition(),
            restartButton
        );

        DrawRectangleRounded(
            restartButton,
            0.2f,
            8,
            hovering ? SKYBLUE : DARKBLUE
        );

        DrawRectangleRoundedLinesEx(
            restartButton,
            0.2f,
            8,
            2,
            SKYBLUE
        );

        DrawText(
            "REINICIAR",
            470, 469, 28, WHITE
        );

        return;
    }

    // Menu inicial.
    if (mode == MODE_MENU) {
        Menu_Draw();
        return;
    }

    // Cenas da historia.
    if (mode == MODE_MEMORY) {
        Memory_Draw();
        return;
    }

    // Telas dos minigames.
    if (mode == MODE_CIPHER) {
        Room_Draw(game.currentRoom);
        Cipher_Draw();
        return;
    }

    if (mode == MODE_DOODLE) {
        Room_Draw(game.currentRoom);
        Doodle_Draw();
        return;
    }

    if (mode == MODE_TERMINAL) {
        Room_Draw(game.currentRoom);
        Terminal_Draw();
        return;
    }

    Room_Draw(game.currentRoom);

    // A porta aparece somente nas salas 1 e 2.
    if (game.currentRoom != 3 && doorClosed.id != 0) {

        float scale = 300.0f / doorClosed.height;

        float doorWidth = doorClosed.width * scale;
        float doorHeight = doorClosed.height * scale;

        DrawTextureEx(
            doorClosed,
            (Vector2){
                vault.x + (vault.width - doorWidth) / 2 - 24,
                vault.y + vault.height - doorHeight
            },
            0.0f,
            scale,
            WHITE
        );
    }

    // Desenho da bateria coletavel.
    if ((game.currentRoom == 2 || game.currentRoom == 3) &&
        !energyCollected) {

        float batteryX = game.currentRoom == 3 ? 687 : 522;
        float batteryY = game.currentRoom == 3 ? 570 : 485;

        DrawCircleGradient(
            (Vector2){batteryX + 28, batteryY + 35},
            48,
            (Color){0, 180, 255, 75},
            BLANK
        );

        DrawRectangleRounded(
            (Rectangle){batteryX, batteryY, 56, 76},
            0.22f, 8,
            (Color){15, 35, 65, 255}
        );

        DrawRectangleRoundedLinesEx(
            (Rectangle){batteryX, batteryY, 56, 76},
            0.22f, 8, 3,
            SKYBLUE
        );

        DrawRectangleRounded(
            (Rectangle){batteryX + 17, batteryY - 8, 22, 10},
            0.3f, 4,
            SKYBLUE
        );

        DrawRectangleRounded(
            (Rectangle){batteryX + 8, batteryY + 8, 40, 60},
            0.15f, 6,
            (Color){0, 105, 165, 255}
        );

        DrawRectangleRoundedLinesEx(
            (Rectangle){batteryX + 8, batteryY + 8, 40, 60},
            0.15f, 6, 2,
            (Color){100, 235, 255, 255}
        );

        DrawText(
            "Z",
            (int)batteryX + 16,
            (int)batteryY + 18,
            36,
            WHITE
        );

        DrawText(
            "+20%",
            (int)batteryX + 1,
            (int)batteryY + 80,
            18,
            SKYBLUE
        );
    }

    Player_Draw(&game.player);

    DrawText(
        "LOGIC CORE",
        50, 25, 25, WHITE
    );

    DrawText(
        TextFormat(
            "SALA %02d / %02d",
            game.currentRoom,
            TOTAL_ROOMS
        ),
        850, 25, 20, LIGHTGRAY
    );

    // Indicador de bateria do BIT.
    Color batteryColor = game.lives > 40
        ? GREEN
        : (game.lives > 20 ? ORANGE : RED);

    DrawText(
        "BATERIA DO BIT",
        50, 642, 16, WHITE
    );

    DrawRectangle(
        50, 672, 180, 14,
        DARKGRAY
    );

    DrawRectangle(
        50,
        672,
        (int)(180 * game.lives / 100.0f),
        14,
        batteryColor
    );

    DrawRectangleLines(
        50, 672, 180, 14,
        WHITE
    );

    DrawText(
        TextFormat("%d%%", game.lives),
        240, 667, 20, batteryColor
    );

    // Mensagem de interacao.
    if (showMessage && !vaultUnlocked) {

        DrawRectangle(
            300, 680, 500, 45,
            DARKBLUE
        );

        DrawText(
            game.currentRoom == 3
                ? "PRESSIONE E PARA ACESSAR O COMPUTADOR"
                : "PRESSIONE E PARA INTERAGIR",
            game.currentRoom == 3 ? 305 : 345,
            692,
            game.currentRoom == 3 ? 17 : 20,
            WHITE
        );
    }

    // Tela de encerramento.
    if (!game.gameRunning) {

        DrawRectangle(
            250, 300, 600, 120,
            DARKBLUE
        );

        DrawText(
            game.lives <= 0
                ? "FIM DE JOGO"
                : campaignCompleted
                    ? "A VERDADE NAO PODE SER APAGADA."
                    : "CAMPANHA CONCLUIDA!",
            300, 345, 25, WHITE
        );
    }
}

void Game_Unload(void)
{
    UnloadCurrentMinigame();

    Menu_Unload();
    Memory_Unload();

    Player_Unload(&game.player);
    Room_Unload();

    if (doorClosed.id != 0) {
        UnloadTexture(doorClosed);
        doorClosed.id = 0;
    }
}
