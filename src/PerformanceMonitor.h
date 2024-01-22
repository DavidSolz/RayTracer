#ifndef PERFORMANCEMONITOR_H
#define PERFORMANCEMONITOR_H

#define BUFFER_SIZE 32

#include "Timer.h"
#include "Logger.h"

#include <algorithm>
#include <cmath>
#include <vector>

class PerformanceMonitor{
private:

    struct PerformanceSample{
        double fpsCount;
        double frameTime;
    };

    struct {
        PerformanceSample mean;
        PerformanceSample variance;
        PerformanceSample deviation;
        PerformanceSample median;
    } statistics;

    Logger * logger;
    std::vector<PerformanceSample> samples;

    char dataBuffer[BUFFER_SIZE] = {0};

    Timer * timer;

    Timepoint samplingStart;
    Timepoint samplingEnd;
    Timepoint lastSamplePoint;

    double samplingDuration;

    void CalculateMean();

    void CalculateVariance();

    void CalculateDeviation();

    void CalculateMedian();

    void CalculateStatistics();

public:
    
    PerformanceMonitor();

    void GatherInformation();

    ~PerformanceMonitor();

};

#endif