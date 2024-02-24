#include "Timer.h"

#include <stdio.h>

Timer::Timer(){
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

double Timer::GetDurationInMiliseconds(const std::chrono::high_resolution_clock::duration& duration){
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

void Timer::TicTac(){

    Timepoint currentTime = GetCurrentTime();
    deltaFrame = GetDurationInSeconds(currentTime - lastTime);
    lastTime = currentTime;
    
    frameCount++;
    accumulatedTime *= (accumulatedTime < 1.0);
    accumulatedTime += deltaFrame;

    lastFrameCount = 1.0f / deltaFrame;
    deltaTime = deltaFrame * 1000.0f;

}

double & Timer::GetDeltaTime() {
    return deltaTime;
}

double & Timer::GetDeltaFrame(){
    return deltaFrame;
}

double & Timer::GetAccumulatedTime(){
    return accumulatedTime;
}

uint32_t & Timer::GetFrameCount(){
    return lastFrameCount;
}

Timer& Timer::GetInstance(){
    static Timer instance;

    return instance;
}

