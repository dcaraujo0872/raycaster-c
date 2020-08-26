#ifndef _RAY_H_
#define _RAY_H_

#include <stdbool.h>
#include <math.h>
#include "player.h"
#include "constants.h"

typedef struct Ray {
    float rayAngle;
    float wallHitX;
    float wallHitY;
    float distance;
    bool wasHitVertical;
    int wallHitContent;
} Ray;

bool isRayFacingDown(float angle) {
    return angle > 0 && angle < M_PI;
}

bool isRayFacingUp(float angle) {
    return !isRayFacingDown(angle);
}

bool isRayFacingRight(float angle) {
    return angle < 0.5 * M_PI || angle > 1.5 * M_PI;
}

bool isRayFacingLeft(float angle) {
    return !isRayFacingRight(angle);
}

void renderRays(SDL_Renderer* renderer, Ray *rays, Player *player) {
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    for (int i = 0; i < NUM_RAYS; i++) {
        SDL_RenderDrawLine(
            renderer,
            MINIMAP_SCALE_FACTOR * player->x,
            MINIMAP_SCALE_FACTOR * player->y,
            MINIMAP_SCALE_FACTOR * (rays + i)->wallHitX,
            MINIMAP_SCALE_FACTOR * (rays + i)->wallHitY
        );
    }
}

#endif