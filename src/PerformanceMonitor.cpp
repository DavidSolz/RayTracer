#include "PerformanceMonitor.h"

PerformanceMonitor::PerformanceMonitor(){

    this->logger.BindOutput("Performance_log.csv");
    this->samplingStart = Timer::GetCurrentTime();
    this->lastSamplePoint = samplingStart;
    this->timer = &Timer::GetInstance();

    logger.Write("fps count ; frametime");
}

void PerformanceMonitor::GatherInformation(){

    Timepoint currentSamplePoint = Timer::GetCurrentTime();
    double duration = Timer::GetDurationInSeconds(currentSamplePoint - lastSamplePoint);

    if( duration < 1.0f)
        return;

    lastSamplePoint = currentSamplePoint;

    PerformanceSample sample = {0};

    uint32_t frameCount = timer->GetFrameCount();
    frameCount = std::max(frameCount, (uint32_t)1);
    double frameTime = timer->GetDeltaTime();

    sample.fpsCount = frameCount;
    sample.frameTime = frameTime;

    samples.emplace_back(sample);

    sprintf(dataBuffer, "%.2f;%f", sample.fpsCount, sample.frameTime);
    logger.Write(dataBuffer);
}

void PerformanceMonitor::CalculateMean(){
    PerformanceSample& mean = statistics.mean;

    uint32_t samplesCount = samples.size()-1;
    samplesCount = std::max((uint32_t)1, samplesCount);

    for(uint32_t id = 1; id < samples.size(); ++id){
        mean.fpsCount += samples[id].fpsCount;
        mean.frameTime += samples[id].frameTime;
    }

    mean.fpsCount/=samplesCount;
    mean.frameTime/=samplesCount;
}

void PerformanceMonitor::CalculateVariance(){
    PerformanceSample& variance = statistics.variance;

    uint32_t samplesCount = samples.size()-1;
    samplesCount = std::max((uint32_t)1, samplesCount);

    for(uint32_t id = 1; id < samples.size(); ++id){
        float deltaFps =  ( statistics.mean.fpsCount - samples[id].fpsCount );
        float deltaFrametime =  ( statistics.mean.frameTime - samples[id].frameTime );

        variance.fpsCount +=  deltaFps * deltaFps;
        variance.frameTime += deltaFrametime * deltaFrametime;

    }

    variance.fpsCount/=samplesCount;
    variance.frameTime/=samplesCount;
}

void PerformanceMonitor::CalculateDeviation(){
    PerformanceSample& deviation = statistics.deviation;

    deviation.fpsCount = sqrt( statistics.variance.fpsCount );
    deviation.frameTime = sqrt( statistics.variance.frameTime );

}

void PerformanceMonitor::CalculateMedian(){
    PerformanceSample& median = statistics.median;

    uint32_t size = samples.size();

    std::sort(samples.begin(), samples.end(), [](PerformanceSample & a, PerformanceSample & b){ return a.fpsCount>b.fpsCount; });

    if(size%2==0){
        median.fpsCount = ( samples[ size>>1 ].fpsCount + samples[ size>>1 + 1 ].fpsCount)/2.0f;
    }else{
        median.fpsCount = samples[ size>>1 + 1].fpsCount;
    }

    std::sort(samples.begin(), samples.end(), [](PerformanceSample & a, PerformanceSample & b){ return a.frameTime>b.frameTime; });

    if(size%2==0){
        median.frameTime = ( samples[ size>>1 ].frameTime + samples[ size>>1 + 1 ].frameTime)/2.0f;
    }else{
        median.frameTime = samples[ size>>1 + 1].frameTime;
    }

}

void PerformanceMonitor::CalculateStatistics(){

    CalculateMean();
    CalculateVariance();
    CalculateDeviation();
    CalculateMedian();
    
}

PerformanceMonitor::~PerformanceMonitor(){

    this->samplingEnd = Timer::GetCurrentTime();
    this->samplingDuration = Timer::GetDurationInSeconds(samplingEnd - samplingStart);

    CalculateStatistics();

    fprintf(stdout, "========[ Statistics ]========\nSampling duration : %f s\nMean : \n\tFPS : %f\n\tframetime : %f \nVariance : \n\tFPS : %f\n\tframetime : %f \nDeviation :\n\tFPS : %f\n\tframetime : %f \nMedian :\n\tFPS : %f\n\tframetime : %f \n",
    samplingDuration,
    statistics.mean.fpsCount, statistics.mean.frameTime,
    statistics.variance.fpsCount, statistics.variance.frameTime,
    statistics.deviation.fpsCount, statistics.deviation.frameTime,
    statistics.median.fpsCount, statistics.median.frameTime
    );

}
