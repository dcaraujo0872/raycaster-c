#include "ray.h"
#include <limits.h>
#include "map.h"
#include "utils.h"
#include "constants.h"

typedef struct GridIntersection {
    float wallHitX;
    float wallHitY;
    bool foundWallHit;
    int content;
} GridIntersection;

void castRay(Ray *ray, Player *player);
float calculateHitDistance(Player *player, float hitX, float hitY, bool foundWallHit);

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

void castAllRays(Ray *rays, Player *player) {
    float rayAngle = player->rotationAngle - (FOV_ANGLE / 2);
    for (int i = 0; i < NUM_RAYS; i++) {
        Ray *ray = (rays + i);;
        ray->angle = normalizeAngle(rayAngle);
        castRay(ray, player);
        rayAngle += FOV_ANGLE / NUM_RAYS;
    }
}

// PRIVATE

void castRay(Ray *ray, Player *player) {
    float xintercept, yintercept;
    float xstep, ystep;

    ///////////////////////////////////////////
    // HORIZONTAL RAY-GRID INTERSECTION CODE
    ///////////////////////////////////////////
    GridIntersection horizontalIntersection = { 0 , 0 };

    // Find the y-coordinate of the closest horizontal grid intersection
    yintercept = floor(player->y / TILE_SIZE) * TILE_SIZE;
    yintercept += isRayFacingDown(ray->angle) ? TILE_SIZE : 0;

    // Find the x-coordinate of the closest horizontal grid intersection
    xintercept = player->x + (yintercept - player->y) / tan(ray->angle);

    // Calculate the increment xstep and ystep
    ystep = TILE_SIZE;
    ystep *= isRayFacingUp(ray->angle) ? -1 : 1;

    xstep = TILE_SIZE / tan(ray->angle);
    xstep *= (isRayFacingLeft(ray->angle) && xstep > 0) ? -1 : 1;
    xstep *= (isRayFacingRight(ray->angle) && xstep < 0) ? -1 : 1;

    float nextHorzTouchX = xintercept;
    float nextHorzTouchY = yintercept;

    // Increment xstep and ystep until we find a wall
    while (inScreenBounds(nextHorzTouchX, nextHorzTouchY)) {
        float xToCheck = nextHorzTouchX;
        float yToCheck = nextHorzTouchY + (isRayFacingUp(ray->angle) ? -1 : 0);

        if (mapHasWallAt(xToCheck, yToCheck)) {
            // found a wall hit
            horizontalIntersection.wallHitX = nextHorzTouchX;
            horizontalIntersection.wallHitY = nextHorzTouchY;
            horizontalIntersection.content = map[(int)floor(yToCheck / TILE_SIZE)][(int)floor(xToCheck / TILE_SIZE)];
            horizontalIntersection.foundWallHit = true;
            break;
        } else {
            nextHorzTouchX += xstep;
            nextHorzTouchY += ystep;
        }
    }

    ///////////////////////////////////////////
    // VERTICAL RAY-GRID INTERSECTION CODE
    ///////////////////////////////////////////
    GridIntersection verticalIntersection = { 0 , 0 };

    // Find the x-coordinate of the closest horizontal grid intersection
    xintercept = floor(player->x / TILE_SIZE) * TILE_SIZE;
    xintercept += isRayFacingRight(ray->angle) ? TILE_SIZE : 0;

    // Find the y-coordinate of the closest horizontal grid intersection
    yintercept = player->y + (xintercept - player->x) * tan(ray->angle);

    // Calculate the increment xstep and ystep
    xstep = TILE_SIZE;
    xstep *= isRayFacingLeft(ray->angle) ? -1 : 1;

    ystep = TILE_SIZE * tan(ray->angle);
    ystep *= (isRayFacingUp(ray->angle) && ystep > 0) ? -1 : 1;
    ystep *= (isRayFacingDown(ray->angle) && ystep < 0) ? -1 : 1;

    float nextVertTouchX = xintercept;
    float nextVertTouchY = yintercept;

    // Increment xstep and ystep until we find a wall
    while (inScreenBounds(nextVertTouchX, nextVertTouchY)) {
        float xToCheck = nextVertTouchX + (isRayFacingLeft(ray->angle) ? -1 : 0);
        float yToCheck = nextVertTouchY;

        if (mapHasWallAt(xToCheck, yToCheck)) {
            // found a wall hit
            verticalIntersection.wallHitX = nextVertTouchX;
            verticalIntersection.wallHitY = nextVertTouchY;
            verticalIntersection.content = map[(int)floor(yToCheck / TILE_SIZE)][(int)floor(xToCheck / TILE_SIZE)];
            verticalIntersection.foundWallHit = true;
            break;
        } else {
            nextVertTouchX += xstep;
            nextVertTouchY += ystep;
        }
    }

    float horizontalHitDistance = calculateHitDistance(player, horizontalIntersection.wallHitX, horizontalIntersection.wallHitY, horizontalIntersection.foundWallHit);
    float verticalHitDistance = calculateHitDistance(player, verticalIntersection.wallHitX, verticalIntersection.wallHitY, verticalIntersection.foundWallHit);

    if (verticalHitDistance < horizontalHitDistance) {
        ray->distance = verticalHitDistance;
        ray->wallHitX = verticalIntersection.wallHitX;
        ray->wallHitY = verticalIntersection.wallHitY;
        ray->wallHitContent = verticalIntersection.content;
        ray->wasHitVertical = true;
    } else {
        ray->distance = horizontalHitDistance;
        ray->wallHitX = horizontalIntersection.wallHitX;
        ray->wallHitY = horizontalIntersection.wallHitY;
        ray->wallHitContent = horizontalIntersection.content;
        ray->wasHitVertical = false;
    }
}

float calculateHitDistance(Player *player, float hitX, float hitY, bool foundWallHit) {
    return foundWallHit
        ? distanceBetweenPoints(player->x, player->y, hitX, hitY)
        : INT_MAX;
}
