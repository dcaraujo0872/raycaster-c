#include "utils.h"
#include <SDL2/SDL.h>
#include "constants.h"

void frameWait(int ticks) {
    int waitTime = FRAME_TIME_LENGTH - (SDL_GetTicks() - ticks);
    if (waitTime > 0 && waitTime <= FRAME_TIME_LENGTH) {
        SDL_Delay(waitTime);
    }
}

float normalizeAngle(float angle) {
    angle = remainder(angle, 2 * M_PI);
    if (angle < 0) {
        angle = (2 * M_PI) + angle;
    }
    return angle;
}

float distanceBetweenPoints(float x1, float y1, float x2, float y2) {
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}
