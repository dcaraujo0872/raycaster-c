#include <stdbool.h>
#include <math.h>

#ifndef CONSTANTS_H
#define CONSTANTS_H

#define PI M_PI
#define TWO_PI 2 * M_PI

#define TILE_SIZE 64
#define MAP_NUM_ROWS 13
#define MAP_NUM_COLS 20

#define MINIMAP_SCALE_FACTOR 0.2 

#define WINDOW_WIDTH (MAP_NUM_COLS * TILE_SIZE)
#define WINDOW_HEIGHT (MAP_NUM_ROWS * TILE_SIZE)

#define FOV_ANGLE (60 * (PI / 180))

#define NUM_RAYS WINDOW_WIDTH

#define FPS 30
#define FRAME_TIME_LENGTH (1000 / FPS)

#endif