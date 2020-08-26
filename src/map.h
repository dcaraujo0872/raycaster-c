#ifndef _MAP_H_
#define _MAP_H_

#include <stdbool.h>
#include <SDL2/SDL.h>
#include "player.h"
#include "constants.h"

extern const int map[MAP_NUM_ROWS][MAP_NUM_COLS];

typedef struct GridIntersection {
    float wallHitX;
    float wallHitY;
    bool foundWallHit;
    int content;
} GridIntersection;

bool mapHasWallAt(float x, float y);
void renderMap(SDL_Renderer* renderer);
bool inScreenBounds(float x, float y);
float calculateHitDistance(Player *player, GridIntersection *intersection);
int mapContentAt(float x, float y);

#endif
