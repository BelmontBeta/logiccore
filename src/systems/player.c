
#include "player.h"

void Player_Init(Player *player, Vector2 startPosition)
{
    player->position = startPosition;
    player->size = (Vector2){40, 50};
    player->speed = 250.0f;
    player->color = SKYBLUE;
}

void Player_Update(Player *player, float dt, Rectangle roomBounds)
{
    // Movimento somente horizontal
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
        player->position.x += player->speed * dt;

    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
        player->position.x -= player->speed * dt;

    // Impede que BIT saia da sala
    if (player->position.x < roomBounds.x)
        player->position.x = roomBounds.x;

    if (player->position.x + player->size.x >
        roomBounds.x + roomBounds.width)
    {
        player->position.x =
            roomBounds.x + roomBounds.width - player->size.x;
    }
}

void Player_Draw(const Player *player)
{
    DrawRectangle(
        (int)player->position.x,
        (int)player->position.y,
        (int)player->size.x,
        (int)player->size.y,
        player->color
    );

    DrawRectangleLines(
        (int)player->position.x,
        (int)player->position.y,
        (int)player->size.x,
        (int)player->size.y,
        WHITE
    );
}
