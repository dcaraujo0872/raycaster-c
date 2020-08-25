#ifndef _RAY_H_
#define _RAY_H_

typedef struct Ray {
    float rayAngle;
    float wallHitX;
    float wallHitY;
    float distance;
    int wasHitVertical;
    int wallHitContent;
} Ray;

#endif