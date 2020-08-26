#ifndef _RAY_H_
#define _RAY_H_

#include <stdbool.h>
#include <math.h>
#include <limits.h>
#include "player.h"
#include "map.h"
#include "utils.h"
#include "constants.h"

typedef struct Ray {
    float angle;
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

void castRay(Ray *ray, Player *player) {
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
    while (nextHorzTouchX >= 0 && nextHorzTouchX <= WINDOW_WIDTH && nextHorzTouchY >= 0 && nextHorzTouchY <= WINDOW_HEIGHT) {
        float xToCheck = nextHorzTouchX;
        float yToCheck = nextHorzTouchY + (isRayFacingUp(ray->angle) ? -1 : 0);

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
    while (nextVertTouchX >= 0 && nextVertTouchX <= WINDOW_WIDTH && nextVertTouchY >= 0 && nextVertTouchY <= WINDOW_HEIGHT) {
        float xToCheck = nextVertTouchX + (isRayFacingLeft(ray->angle) ? -1 : 0);
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
        ? distanceBetweenPoints(player->x, player->y, horzWallHitX, horzWallHitY)
        : INT_MAX;
    float vertHitDistance = foundVertWallHit
        ? distanceBetweenPoints(player->x, player->y, vertWallHitX, vertWallHitY)
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

void castAllRays(Ray *rays, Player *player) {
    float rayAngle = player->rotationAngle - (FOV_ANGLE / 2);
    for (int i = 0; i < NUM_RAYS; i++) {
        Ray *ray = (rays + i);;
        ray->angle = normalizeAngle(rayAngle);
        castRay(ray, player);
        rayAngle += FOV_ANGLE / NUM_RAYS;
    }
}

#endif