#include "Timer.h"

#include <stdio.h>

static Timer* instance;

Timer::Timer(){
    this->timeScale = 1.0f;
    this->lastTime = glfwGetTime();
    this->deltaTime = 1/60.0f;
    this->frameCount = 0;
}

void Timer::TicTac(){
    double currentTime = glfwGetTime();
    frameCount++;
    double delta = currentTime - lastTime;
    if(delta >= 1.0f){
        deltaTime = timeScale/frameCount;
        lastFrameCount = frameCount;
        frameCount = 0;
        lastTime = currentTime;
    }
}

double Timer::GetDeltaTime() const{
    return deltaTime;
}

uint32_t Timer::GetFrameCount() const{
    return lastFrameCount;
}

void Timer::SetTimeScale(const double& _timeScale){
        this->timeScale = _timeScale;
}

Timer* Timer::GetInstance(){
    if(!instance)
        instance = new Timer();

    return instance;
}

Timer::~Timer(){
    delete instance;
}
