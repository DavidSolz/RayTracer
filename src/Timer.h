#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include <chrono>

typedef std::chrono::high_resolution_clock::time_point Timepoint;

class Timer {

private:

    Timepoint lastTime;

    double accumulatedTime;
    double deltaFrame;
    double deltaTime;

    uint32_t lastFrameCount;
    uint32_t frameCount;

    Timer();

    
public:

    void TicTac();

    double & GetDeltaTime();

    double & GetDeltaFrame();

    uint32_t & GetFrameCount();

    double & GetAccumulatedTime();

    static Timer& GetInstance();

    static Timepoint GetCurrentTime();

    static double GetDurationInSeconds(const std::chrono::high_resolution_clock::duration& duration);


};

#endif
