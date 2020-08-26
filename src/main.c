#include <SDL2/SDL.h>
#include "ray.h"
#include "player.h"
#include "map.h"
#include "upng.h"
#include "utils.h"
#include "constants.h"

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
int isGameRunning = false;
int ticksLastFrame;
uint32_t *colorBuffer = NULL;
SDL_Texture *colorBufferTexture;
uint32_t *wallTexture;
upng_t *pngTexture;

Player player;
Ray rays[NUM_RAYS];

int initializeWindow(void);
void setup(void);
void processInput(void);
void update(void);
void render(void);
void destroyWindow(void);
void clearColorBuffer(uint32_t color);
void renderColorBuffer(void);
void generate3DProjection(void);
void renderCeiling(int wallTop, int rayIndex);
void renderWall(int wallTop, int wallBottom, int wallHeight, int rayIndex);
void renderFloor(int wallBottom, int rayIndex);

int main() {
    isGameRunning = initializeWindow();
    setup();
    while (isGameRunning) {
        processInput();
        update();
        render();
    }
    destroyWindow();
    return 0;
}

// GAME LOOP

int initializeWindow(void) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error initializing SDL.\n");
        return false;
    }
    window = SDL_CreateWindow(
        NULL,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_BORDERLESS
    );
    if (!window) {
        fprintf(stderr, "Error creating SDL window.\n");
        return false;
    }
    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        fprintf(stderr, "Error creating SDL renderer.\n");
        return false;
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    return true;
}

void setup(void) {
    player.x = WINDOW_WIDTH / 2;
    player.y = WINDOW_HEIGHT / 2;
    player.width = 1;
    player.height = 1;
    player.turnDirection = 0;
    player.walkDirection = 0;
    player.rotationAngle = M_PI / 2;
    player.walkSpeed = 100;
    player.turnSpeed = 45 * (M_PI / 180);

    // allocate the total amount of bytes in memory to hold our colorbuffer
    colorBuffer = (uint32_t*) malloc(sizeof(uint32_t) * (uint32_t)WINDOW_WIDTH * (uint32_t)WINDOW_HEIGHT);

    // create an SDL_Texture to display the colorbuffer
    colorBufferTexture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STREAMING,
        WINDOW_WIDTH,
        WINDOW_HEIGHT
    );

    // allocate the total amount of bytes in memory to hold our wall texture
    wallTexture = (uint32_t*) malloc(sizeof(uint32_t) * (uint32_t)TEXTURE_WIDTH * (uint32_t)TEXTURE_HEIGHT);

    // load an external texture using the upng library to decode the file
    pngTexture = upng_new_from_file(WOOD_TEXTURE_FILEPATH);
    if (pngTexture != NULL) {
        upng_decode(pngTexture);
        if (upng_get_error(pngTexture) == UPNG_EOK) {
            wallTexture = (uint32_t*) upng_get_buffer(pngTexture);
        }
    }
}

void processInput(void) {
    SDL_Event event;
    SDL_PollEvent(&event);
    switch (event.type) {
        case SDL_QUIT: {
            isGameRunning = false;
            break;
        }
        case SDL_KEYDOWN: {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                isGameRunning = false;
            }
            if (event.key.keysym.sym == SDLK_UP) {
                player.walkDirection = +1;
            }
            if (event.key.keysym.sym == SDLK_DOWN) {
                player.walkDirection = -1;
            }
            if (event.key.keysym.sym == SDLK_RIGHT) {
                player.turnDirection = +1;
            }
            if (event.key.keysym.sym == SDLK_LEFT) {
                player.turnDirection = -1;
            }
            break;
        }
        case SDL_KEYUP: {
            if (event.key.keysym.sym == SDLK_UP) {
                player.walkDirection = 0;
            }
            if (event.key.keysym.sym == SDLK_DOWN) {
                player.walkDirection = 0;
            }
            if (event.key.keysym.sym == SDLK_RIGHT) {
                player.turnDirection = 0;
            }
            if (event.key.keysym.sym == SDLK_LEFT) {
                player.turnDirection = 0;
            }
            break;
        }
    }
}

void update(void) {
    frameWait(ticksLastFrame);
    float deltaTime = (SDL_GetTicks() - ticksLastFrame) / 1000.0f;
    ticksLastFrame = SDL_GetTicks();
    movePlayer(&player, deltaTime);
    castAllRays(rays, &player);
}

void render(void) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    generate3DProjection();
    renderColorBuffer();
    clearColorBuffer(0xFF000000);
    renderMap(renderer);
    renderRays(renderer, rays, &player);
    renderPlayer(renderer, &player);
    SDL_RenderPresent(renderer);
}

void destroyWindow(void) {
    free(wallTexture);
    free(colorBuffer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

// RENDERING

void clearColorBuffer(uint32_t color) {
    for (int x = 0; x < WINDOW_WIDTH; x++) {
        for (int y = 0; y < WINDOW_HEIGHT; y++) {
            colorBuffer[(WINDOW_WIDTH * y) + x] = color;
        }
    }
}

void renderColorBuffer(void) {
    SDL_UpdateTexture(
        colorBufferTexture,
        NULL,
        colorBuffer,
        (int)((uint32_t)WINDOW_WIDTH * sizeof(uint32_t))
    );
    SDL_RenderCopy(renderer, colorBufferTexture, NULL, NULL);
}

void generate3DProjection(void) {
    for (int i = 0; i < NUM_RAYS; i++) {
        float perpendicularDistance = rays[i].distance * cos(rays[i].angle - player.rotationAngle);
        float projectionPlaneDistance = (WINDOW_WIDTH / 2) / tan(FOV_ANGLE / 2);
        float projectedWallHeight = (TILE_SIZE / perpendicularDistance) * projectionPlaneDistance;

        int wallStripHeight = (int)projectedWallHeight;

        int wallTopPixel = (WINDOW_HEIGHT / 2) - (wallStripHeight / 2);
        wallTopPixel = wallTopPixel < 0 ? 0 : wallTopPixel;

        int wallBottomPixel = (WINDOW_HEIGHT / 2) + (wallStripHeight / 2);
        wallBottomPixel = wallBottomPixel > WINDOW_HEIGHT ? WINDOW_HEIGHT : wallBottomPixel;

        renderCeiling(wallTopPixel, i);
        renderWall(wallTopPixel, wallBottomPixel, wallStripHeight, i);
        renderFloor(wallBottomPixel, i);
    }
}

void renderCeiling(int wallTop, int rayIndex) {
    for (int y = 0; y < wallTop; y++) {
        colorBuffer[(WINDOW_WIDTH * y) + rayIndex] = 0xFF444444;
    }
}

void renderWall(int wallTop, int wallBottom, int wallHeight, int rayIndex) {
    int textureOffsetX;
    if (rays[rayIndex].wasHitVertical) {
        textureOffsetX = (int)rays[rayIndex].wallHitY % TILE_SIZE;
    }
    else {
        textureOffsetX = (int)rays[rayIndex].wallHitX % TILE_SIZE;
    }

    // render the wall from wallTopPixel to wallBottomPixel
    for (int y = wallTop; y < wallBottom; y++) {
        int distanceFromTop = y + (wallHeight / 2) - (WINDOW_HEIGHT / 2);
        int textureOffsetY = distanceFromTop * ((float)TEXTURE_HEIGHT / wallHeight);

        // set the color of the wall texture based on the color from the texture in memory
        uint32_t texelColor = wallTexture[(TEXTURE_WIDTH * textureOffsetY) + textureOffsetX];
        colorBuffer[(WINDOW_WIDTH * y) + rayIndex] = texelColor;
    }
}

void renderFloor(int wallBottom, int rayIndex) {
    for (int y = wallBottom; y < WINDOW_HEIGHT; y++) {
        colorBuffer[(WINDOW_WIDTH * y) + rayIndex] = 0xFF888888;
    }
}
