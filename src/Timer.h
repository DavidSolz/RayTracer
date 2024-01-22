#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include <chrono>

typedef std::chrono::high_resolution_clock::time_point Timepoint;

class Timer {

private:

    double timeScale;
    Timepoint lastTime;
    double deltaTime;

    uint32_t lastFrameCount;
    uint32_t frameCount;

    Timer();

    
public:

    void TicTac();

    void SetTimeScale(const double& _timeScale);

    double GetDeltaTime() const;

    uint32_t GetFrameCount() const;

    static Timer* GetInstance();

    static Timepoint GetCurrentTime();

    static double GetDurationInSeconds(const std::chrono::high_resolution_clock::duration& duration);

    ~Timer();

};

#endif
