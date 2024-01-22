#include "Timer.h"

#include <stdio.h>

static Timer* instance;

Timer::Timer(){
    this->timeScale = 1.0f;
    this->lastTime = GetCurrentTime();
    this->deltaTime = 1/60.0f;
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
    double delta = GetDurationInSeconds(currentTime - lastTime);
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
