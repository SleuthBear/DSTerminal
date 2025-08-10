//
// Created by Dylan Beaumont on 8/8/2025.
//

#ifndef KEYSTATE_H
#define KEYSTATE_H

#define DS_DIR 0
#define DS_FILE 1
typedef struct {
    double left = 10;
    double right = 10;
    double backspace = 10;
    double enter = 10;
    double interval = 0.1f;
    void addDeltaTime(double deltaTime) {
        left+=deltaTime;
        right+=deltaTime;
        backspace+=deltaTime;
        enter+=deltaTime;
    }
} KeyState;

#endif
