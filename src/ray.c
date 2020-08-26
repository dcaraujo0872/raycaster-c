#include "ray.h"
#include "map.h"
#include "utils.h"
#include "constants.h"

void castRay(Ray *ray, Player *player);
GridIntersection horizontalGridIntersection(Ray *ray, Player *player);
GridIntersection verticalGridIntersection(Ray *ray, Player *player);
bool isRayFacingDown(float angle);
bool isRayFacingUp(float angle);
bool isRayFacingRight(float angle);
bool isRayFacingLeft(float angle);

void renderRays(SDL_Renderer *renderer, Ray *rays, Player *player) {
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
    GridIntersection horizontalIntersection = horizontalGridIntersection(ray, player);
    GridIntersection verticalIntersection = verticalGridIntersection(ray, player);

    float horizontalHitDistance = calculateHitDistance(player, &horizontalIntersection);
    float verticalHitDistance = calculateHitDistance(player, &verticalIntersection);

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

GridIntersection horizontalGridIntersection(Ray *ray, Player *player) {
    GridIntersection intersection = { 0 , 0 };

    float yIntercept = floor(player->y / TILE_SIZE) * TILE_SIZE;
    yIntercept += isRayFacingDown(ray->angle) ? TILE_SIZE : 0;

    float xIntercept = player->x + (yIntercept - player->y) / tan(ray->angle);

    float yStep = TILE_SIZE;
    yStep *= isRayFacingUp(ray->angle) ? -1 : 1;

    float xStep = TILE_SIZE / tan(ray->angle);
    xStep *= (isRayFacingLeft(ray->angle) && xStep > 0) ? -1 : 1;
    xStep *= (isRayFacingRight(ray->angle) && xStep < 0) ? -1 : 1;

    float nextTouchX = xIntercept;
    float nextTouchY = yIntercept;

    // Increment xStep and yStep until we find a wall
    while (inScreenBounds(nextTouchX, nextTouchY)) {
        float xToCheck = nextTouchX;
        float yToCheck = nextTouchY + (isRayFacingUp(ray->angle) ? -1 : 0);

        if (mapHasWallAt(xToCheck, yToCheck)) {
            // found a wall hit
            intersection.wallHitX = nextTouchX;
            intersection.wallHitY = nextTouchY;
            intersection.content = mapContentAt(xToCheck, yToCheck);
            intersection.foundWallHit = true;
            break;
        } else {
            nextTouchX += xStep;
            nextTouchY += yStep;
        }
    }
    return intersection;
}

GridIntersection verticalGridIntersection(Ray *ray, Player *player) {
    GridIntersection intersection = { 0 , 0 };

    float xIntercept = floor(player->x / TILE_SIZE) * TILE_SIZE;
    xIntercept += isRayFacingRight(ray->angle) ? TILE_SIZE : 0;

    float yIntercept = player->y + (xIntercept - player->x) * tan(ray->angle);

    float xStep = TILE_SIZE;
    xStep *= isRayFacingLeft(ray->angle) ? -1 : 1;

    float yStep = TILE_SIZE * tan(ray->angle);
    yStep *= (isRayFacingUp(ray->angle) && yStep > 0) ? -1 : 1;
    yStep *= (isRayFacingDown(ray->angle) && yStep < 0) ? -1 : 1;

    float nextTouchX = xIntercept;
    float nextTouchY = yIntercept;

    while (inScreenBounds(nextTouchX, nextTouchY)) {
        float xToCheck = nextTouchX + (isRayFacingLeft(ray->angle) ? -1 : 0);
        float yToCheck = nextTouchY;

        if (mapHasWallAt(xToCheck, yToCheck)) {
            intersection.wallHitX = nextTouchX;
            intersection.wallHitY = nextTouchY;
            intersection.content = mapContentAt(xToCheck, yToCheck);
            intersection.foundWallHit = true;
            break;
        } else {
            nextTouchX += xStep;
            nextTouchY += yStep;
        }
    }
    return intersection;
}

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
