#include "player.h"
#include "constants.h"
#include "map.h"

void movePlayer(Player *player, float deltaTime) {
    player->rotationAngle += player->turnDirection * player->turnSpeed * deltaTime;
    float moveStep = player->walkDirection * player->walkSpeed * deltaTime;

    float newPlayerX = player->x + cos(player->rotationAngle) * moveStep;
    float newPlayerY = player->y + sin(player->rotationAngle) * moveStep;

    if (!mapHasWallAt(newPlayerX, newPlayerY)) {
        player->x = newPlayerX;
        player->y = newPlayerY;
    }
}

void renderPlayer(SDL_Renderer *renderer, Player *player) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect playerRect = {
        player->x * MINIMAP_SCALE_FACTOR,
        player->y * MINIMAP_SCALE_FACTOR,
        player->width * MINIMAP_SCALE_FACTOR,
        player->height * MINIMAP_SCALE_FACTOR
    };
    SDL_RenderFillRect(renderer, &playerRect);
    SDL_RenderDrawLine(
        renderer,
        MINIMAP_SCALE_FACTOR * player->x,
        MINIMAP_SCALE_FACTOR * player->y,
        MINIMAP_SCALE_FACTOR * player->x + cos(player->rotationAngle) * 40,
        MINIMAP_SCALE_FACTOR * player->y + sin(player->rotationAngle) * 40
    );
}