#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#include <stdbool.h>
#include <math.h>

#define TILE_SIZE 64
#define MAP_NUM_ROWS 13
#define MAP_NUM_COLS 20
#define NUM_TEXTURES 8

#define MINIMAP_SCALE_FACTOR 0.2

#define WINDOW_WIDTH (MAP_NUM_COLS * TILE_SIZE)
#define WINDOW_HEIGHT (MAP_NUM_ROWS * TILE_SIZE)

#define TEXTURE_WIDTH 64
#define TEXTURE_HEIGHT 64

#define FOV_ANGLE (60 * (M_PI / 180))

#define NUM_RAYS WINDOW_WIDTH

#define FPS 30
#define FRAME_TIME_LENGTH (1000 / FPS)

#define REDBRICK_TEXTURE_FILEPATH "./images/redbrick.png"
#define PURPLESTONE_TEXTURE_FILEPATH "./images/purplestone.png"
#define MOSSYSTONE_TEXTURE_FILEPATH "./images/mossystone.png"
#define GRAYSTONE_TEXTURE_FILEPATH "./images/graystone.png"
#define COLORSTONE_TEXTURE_FILEPATH "./images/colorstone.png"
#define BLUESTONE_TEXTURE_FILEPATH "./images/bluestone.png"
#define WOOD_TEXTURE_FILEPATH "./images/wood.png"
#define EAGLE_TEXTURE_FILEPATH "./images/eagle.png"
#define PIKUMA_TEXTURE_FILEPATH "./images/pikuma.png"

#endif
