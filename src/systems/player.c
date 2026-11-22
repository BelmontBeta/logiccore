
#include "player.h"

#include <math.h>

#define SPRITE_PATH "assets/sprites/bit/"

void Player_Init(Player *player, Vector2 startPosition)
{
    player->position = startPosition;
    player->size = (Vector2){128, 128};
    player->speed = 250.0f;
    player->color = WHITE;

    player->idleFrames[0] = LoadTexture(
        SPRITE_PATH "bitparado1.png"
    );

    player->idleFrames[1] = LoadTexture(
        SPRITE_PATH "bitparado2.png"
    );

    player->walkFrames[0] = LoadTexture(
        SPRITE_PATH "bitandando1.png"
    );

    player->walkFrames[1] = LoadTexture(
        SPRITE_PATH "bitandando2.png"
    );

    player->walkFrames[2] = LoadTexture(
        SPRITE_PATH "bitandando3.png"
    );

    player->currentFrame = 0;
    player->animationTimer = 0.0f;
    player->moving = false;
    player->facingLeft = false;
}

void Player_Update(Player *player, float dt, Rectangle roomBounds)
{
    float movement = 0.0f;

    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
        movement += 1.0f;

    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
        movement -= 1.0f;

    player->moving = movement != 0.0f;

    static bool previousMoving = false;

    if (player->moving != previousMoving)
    {
        player->currentFrame = 0;
        player->animationTimer = 0.0f;
        previousMoving = player->moving;
    }

    if (movement != 0.0f)
    {
        player->position.x += movement * player->speed * dt;
        player->facingLeft = movement < 0.0f;
    }

    if (player->position.x < roomBounds.x)
        player->position.x = roomBounds.x;

    if (player->position.x + player->size.x >
        roomBounds.x + roomBounds.width)
    {
        player->position.x =
            roomBounds.x + roomBounds.width - player->size.x;
    }

    player->animationTimer += dt;

    float frameDuration = player->moving ? 0.12f : 0.45f;

    if (player->animationTimer >= frameDuration)
    {
        player->animationTimer = 0.0f;

        int frameCount = player->moving ? 3 : 2;

        player->currentFrame =
            (player->currentFrame + 1) % frameCount;
    }
}

void Player_Draw(const Player *player)
{
    Texture2D texture;

    if (player->moving)
        texture = player->walkFrames[player->currentFrame];
    else
        texture = player->idleFrames[player->currentFrame];

    Rectangle source = {
        0,
        0,
        (float)texture.width,
        (float)texture.height
    };

    if (player->facingLeft)
        source.width = -source.width;

    Rectangle destination = {
        player->position.x,
        player->position.y,
        player->size.x,
        player->size.y
    };

    Vector2 origin = {0, 0};

    DrawTexturePro(
        texture,
        source,
        destination,
        origin,
        0.0f,
        WHITE
    );
}

void Player_Unload(Player *player)
{
    for (int i = 0; i < 2; i++)
    {
        UnloadTexture(player->idleFrames[i]);
    }

    for (int i = 0; i < 3; i++)
    {
        UnloadTexture(player->walkFrames[i]);
    }
}
