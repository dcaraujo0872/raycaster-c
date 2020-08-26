#include <stdio.h>
#include <limits.h>
#include <SDL2/SDL.h>
#include "upng.h"
#include "constants.h"
#include "ray.h"
#include "player.h"
#include "map.h"
#include "utils.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
int isGameRunning = false;
int ticksLastFrame;
uint32_t* colorBuffer = NULL;
SDL_Texture* colorBufferTexture;
uint32_t* wallTexture;
upng_t* pngTexture;

Player player;
Ray rays[NUM_RAYS];

void castRay(Ray *ray) {
    float xintercept, yintercept;
    float xstep, ystep;

    ///////////////////////////////////////////
    // HORIZONTAL RAY-GRID INTERSECTION CODE
    ///////////////////////////////////////////
    int foundHorzWallHit = false;
    float horzWallHitX = 0;
    float horzWallHitY = 0;
    int horzWallContent = 0;

    // Find the y-coordinate of the closest horizontal grid intersection
    yintercept = floor(player.y / TILE_SIZE) * TILE_SIZE;
    yintercept += isRayFacingDown(ray->rayAngle) ? TILE_SIZE : 0;

    // Find the x-coordinate of the closest horizontal grid intersection
    xintercept = player.x + (yintercept - player.y) / tan(ray->rayAngle);

    // Calculate the increment xstep and ystep
    ystep = TILE_SIZE;
    ystep *= isRayFacingUp(ray->rayAngle) ? -1 : 1;

    xstep = TILE_SIZE / tan(ray->rayAngle);
    xstep *= (isRayFacingLeft(ray->rayAngle) && xstep > 0) ? -1 : 1;
    xstep *= (isRayFacingRight(ray->rayAngle) && xstep < 0) ? -1 : 1;

    float nextHorzTouchX = xintercept;
    float nextHorzTouchY = yintercept;

    // Increment xstep and ystep until we find a wall
    while (nextHorzTouchX >= 0 && nextHorzTouchX <= WINDOW_WIDTH && nextHorzTouchY >= 0 && nextHorzTouchY <= WINDOW_HEIGHT) {
        float xToCheck = nextHorzTouchX;
        float yToCheck = nextHorzTouchY + (isRayFacingUp(ray->rayAngle) ? -1 : 0);

        if (mapHasWallAt(xToCheck, yToCheck)) {
            // found a wall hit
            horzWallHitX = nextHorzTouchX;
            horzWallHitY = nextHorzTouchY;
            horzWallContent = map[(int)floor(yToCheck / TILE_SIZE)][(int)floor(xToCheck / TILE_SIZE)];
            foundHorzWallHit = true;
            break;
        } else {
            nextHorzTouchX += xstep;
            nextHorzTouchY += ystep;
        }
    }

    ///////////////////////////////////////////
    // VERTICAL RAY-GRID INTERSECTION CODE
    ///////////////////////////////////////////
    int foundVertWallHit = false;
    float vertWallHitX = 0;
    float vertWallHitY = 0;
    int vertWallContent = 0;

    // Find the x-coordinate of the closest horizontal grid intersection
    xintercept = floor(player.x / TILE_SIZE) * TILE_SIZE;
    xintercept += isRayFacingRight(ray->rayAngle) ? TILE_SIZE : 0;

    // Find the y-coordinate of the closest horizontal grid intersection
    yintercept = player.y + (xintercept - player.x) * tan(ray->rayAngle);

    // Calculate the increment xstep and ystep
    xstep = TILE_SIZE;
    xstep *= isRayFacingLeft(ray->rayAngle) ? -1 : 1;

    ystep = TILE_SIZE * tan(ray->rayAngle);
    ystep *= (isRayFacingUp(ray->rayAngle) && ystep > 0) ? -1 : 1;
    ystep *= (isRayFacingDown(ray->rayAngle) && ystep < 0) ? -1 : 1;

    float nextVertTouchX = xintercept;
    float nextVertTouchY = yintercept;

    // Increment xstep and ystep until we find a wall
    while (nextVertTouchX >= 0 && nextVertTouchX <= WINDOW_WIDTH && nextVertTouchY >= 0 && nextVertTouchY <= WINDOW_HEIGHT) {
        float xToCheck = nextVertTouchX + (isRayFacingLeft(ray->rayAngle) ? -1 : 0);
        float yToCheck = nextVertTouchY;

        if (mapHasWallAt(xToCheck, yToCheck)) {
            // found a wall hit
            vertWallHitX = nextVertTouchX;
            vertWallHitY = nextVertTouchY;
            vertWallContent = map[(int)floor(yToCheck / TILE_SIZE)][(int)floor(xToCheck / TILE_SIZE)];
            foundVertWallHit = true;
            break;
        } else {
            nextVertTouchX += xstep;
            nextVertTouchY += ystep;
        }
    }

    // Calculate both horizontal and vertical hit distances and choose the smallest one
    float horzHitDistance = foundHorzWallHit
        ? distanceBetweenPoints(player.x, player.y, horzWallHitX, horzWallHitY)
        : INT_MAX;
    float vertHitDistance = foundVertWallHit
        ? distanceBetweenPoints(player.x, player.y, vertWallHitX, vertWallHitY)
        : INT_MAX;

    if (vertHitDistance < horzHitDistance) {
        ray->distance = vertHitDistance;
        ray->wallHitX = vertWallHitX;
        ray->wallHitY = vertWallHitY;
        ray->wallHitContent = vertWallContent;
        ray->wasHitVertical = true;
    } else {
        ray->distance = horzHitDistance;
        ray->wallHitX = horzWallHitX;
        ray->wallHitY = horzWallHitY;
        ray->wallHitContent = horzWallContent;
        ray->wasHitVertical = false;
    }
}

void castAllRays(void) {
    float rayAngle = player.rotationAngle - (FOV_ANGLE / 2);
    for (int i = 0; i < NUM_RAYS; i++) {
        rays[i].rayAngle = normalizeAngle(rayAngle);
        castRay(&rays[i]);
        rayAngle += FOV_ANGLE / NUM_RAYS;
    }
}

void generate3DProjection(void) {
    for (int i = 0; i < NUM_RAYS; i++) {
        float perpDistance = rays[i].distance * cos(rays[i].rayAngle - player.rotationAngle);
        float distanceProjPlane = (WINDOW_WIDTH / 2) / tan(FOV_ANGLE / 2);
        float projectedWallHeight = (TILE_SIZE / perpDistance) * distanceProjPlane;

        int wallStripHeight = (int)projectedWallHeight;

        int wallTopPixel = (WINDOW_HEIGHT / 2) - (wallStripHeight / 2);
        wallTopPixel = wallTopPixel < 0 ? 0 : wallTopPixel;

        int wallBottomPixel = (WINDOW_HEIGHT / 2) + (wallStripHeight / 2);
        wallBottomPixel = wallBottomPixel > WINDOW_HEIGHT ? WINDOW_HEIGHT : wallBottomPixel;

        // set the color of the ceiling
        for (int y = 0; y < wallTopPixel; y++)
            colorBuffer[(WINDOW_WIDTH * y) + i] = 0xFF444444;

        int textureOffsetX;
        if (rays[i].wasHitVertical) {
            textureOffsetX = (int)rays[i].wallHitY % TILE_SIZE;
        }
        else {
            textureOffsetX = (int)rays[i].wallHitX % TILE_SIZE;
        }

        // get the correct texture id number from the map content
        int texNum = rays[i].wallHitContent - 1;

        // render the wall from wallTopPixel to wallBottomPixel
        for (int y = wallTopPixel; y < wallBottomPixel; y++) {
            int distanceFromTop = y + (wallStripHeight / 2) - (WINDOW_HEIGHT / 2);
            int textureOffsetY = distanceFromTop * ((float)TEXTURE_HEIGHT / wallStripHeight);

            // set the color of the wall texture based on the color from the texture in memory
            uint32_t texelColor = wallTexture[(TEXTURE_WIDTH * textureOffsetY) + textureOffsetX];
            colorBuffer[(WINDOW_WIDTH * y) + i] = texelColor;
        }

        // set the color of the floor
        for (int y = wallBottomPixel; y < WINDOW_HEIGHT; y++) {
            colorBuffer[(WINDOW_WIDTH * y) + i] = 0xFF888888;
        }
    }
}

int initializeWindow(void);
void setup(void);
void processInput(void);
void update(void);
void render(void);
void destroyWindow(void);
void clearColorBuffer(uint32_t color);
void renderColorBuffer(void);

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
    pngTexture = upng_new_from_file(PIKUMA_TEXTURE_FILEPATH);
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
    // waste some time until we reach the target frame time length
    while (!SDL_TICKS_PASSED(SDL_GetTicks(), ticksLastFrame + FRAME_TIME_LENGTH));

    float deltaTime = (SDL_GetTicks() - ticksLastFrame) / 1000.0f;

    ticksLastFrame = SDL_GetTicks();

    movePlayer(&player, deltaTime);
    castAllRays();
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
