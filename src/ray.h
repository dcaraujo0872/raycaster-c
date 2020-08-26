#ifndef _RAY_H_
#define _RAY_H_

#include <stdbool.h>
#include "player.h"

typedef struct Ray {
    float angle;
    float wallHitX;
    float wallHitY;
    float distance;
    bool wasHitVertical;
    int wallHitContent;
} Ray;

bool isRayFacingDown(float angle);
bool isRayFacingUp(float angle);
bool isRayFacingRight(float angle);
bool isRayFacingLeft(float angle);
void renderRays(SDL_Renderer* renderer, Ray *rays, Player *player);
void castAllRays(Ray *rays, Player *player);

#endif
