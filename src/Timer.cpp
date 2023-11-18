#include "Timer.h"

#include <stdio.h>

static Timer* instance;

Timer::Timer(){
    this->timeScale = 1.0f;
    this->lastTime = glfwGetTime();
}

void Timer::TicTac(){
    float currentTime = glfwGetTime();
    frameCount++;
    float delta = currentTime - lastTime;
    if(delta >= 1.0f){
        deltaTime = timeScale/frameCount;
        fprintf(stdout, "frametime : %fms\r", 1000.0/frameCount);
        frameCount = 0;
        lastTime = currentTime;
    }
}

float Timer::GetDeltaTime(){
    return deltaTime;
}

void Timer::SetTimeScale(const float& _timeScale){
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