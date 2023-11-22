#ifndef TIMER_H
#define TIMER_H

#include <GLFW/glfw3.h>
#include <stdio.h>

class Timer {

private:

    double timeScale;
    double lastTime;
    double deltaTime;
    unsigned int frameCount;

    Timer();

public:

    void TicTac();

    void SetTimeScale(const double& _timeScale);

    double GetDeltaTime();

    static Timer* GetInstance();

    ~Timer();

};

#endif
