#ifndef _MAP_H_
#define _MAP_H_

#include <stdbool.h>
#include <SDL2/SDL.h>
#include "constants.h"

extern const int map[MAP_NUM_ROWS][MAP_NUM_COLS];

bool mapHasWallAt(float x, float y);
void renderMap(SDL_Renderer* renderer);

#endif
