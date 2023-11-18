#ifndef TIMER_H
#define TIMER_H

#include <GLFW/glfw3.h>
#include <stdio.h>

class Timer {

private:

    float timeScale;
    float lastTime;
    float deltaTime;
    unsigned int frameCount;

    Timer();

public:

    void TicTac();

    void SetTimeScale(const float& _timeScale);

    float GetDeltaTime();

    static Timer* GetInstance();

    ~Timer();

};

#endif