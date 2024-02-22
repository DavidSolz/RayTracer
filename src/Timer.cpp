#include "Timer.h"

#include <stdio.h>

Timer::Timer(){
    this->timeScale = 1.0f;
    this->lastTime = GetCurrentTime();
    this->deltaTime = 1.0f/60.0f;
    this->frameCount = 60;
}

std::chrono::high_resolution_clock::time_point Timer::GetCurrentTime() {
    return std::chrono::high_resolution_clock::now();
}

double Timer::GetDurationInSeconds(const std::chrono::high_resolution_clock::duration& duration) {
    return std::chrono::duration_cast<std::chrono::duration<double>>(duration).count();
}

void Timer::TicTac(){
    Timepoint currentTime = GetCurrentTime();
    frameCount++;
    deltaFrame = GetDurationInSeconds(currentTime - lastTime);
    if(deltaFrame >= 1.0f){
        deltaTime = 1000.0f/frameCount;
        lastFrameCount = frameCount;
        frameCount = 0;
        lastTime = currentTime;
    }
}

double & Timer::GetDeltaTime() {
    return deltaTime;
}

double & Timer::GetDeltaFrame(){
    return deltaFrame;
}

uint32_t & Timer::GetFrameCount(){
    return lastFrameCount;
}

void Timer::SetTimeScale(const double& _timeScale){
    this->timeScale = _timeScale;
}

Timer& Timer::GetInstance(){
    static Timer instance;

    return instance;
}

