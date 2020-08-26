#ifndef _PLAYER_H_
#define _PLAYER_H_

#include <SDL2/SDL.h>

typedef struct Player {
    float x;
    float y;
    float width;
    float height;
    int turnDirection; // -1 for left, +1 for right
    int walkDirection; // -1 for back, +1 for front
    float rotationAngle;
    float walkSpeed;
    float turnSpeed;
} Player;

void movePlayer(Player *player, float deltaTime);
void renderPlayer(SDL_Renderer *renderer, Player *player);

#endif
