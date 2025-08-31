//
// Created by Dylan Beaumont on 8/8/2025.
//

#ifndef KEYSTATE_H
#define KEYSTATE_H

#define DS_DIR 0
#define DS_FILE 1
struct KeyState{
    double left = 10;
    double right = 10;
    double up = 10;
    double backspace = 10;
    double enter = 10;
    double space = 10;
    double tab = 10;
    double escape = 10;
    double interval = 0.1f;
    void addDeltaTime(double deltaTime) {
        left+=deltaTime;
        right+=deltaTime;
        up+=deltaTime;
        backspace+=deltaTime;
        enter+=deltaTime;
        space+=deltaTime;
        tab+=deltaTime;
        escape+=deltaTime;
    }
};

#endif
