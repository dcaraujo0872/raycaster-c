#ifndef _RAY_H_
#define _RAY_H_

#include <stdbool.h>
#include <math.h>

typedef struct Ray {
    float rayAngle;
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

#endif