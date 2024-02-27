#include "ThreadedShader.h"



ThreadedShader::ThreadedShader(RenderingContext * _context) : ComputeShader(_context){

    numThreads = std::thread::hardware_concurrency();
    bool isHyperThreadinEnabled = CheckForHyperthreading();

    context->loggingService.Write(MessageType::INFO, "Discovered %d logic cores", numThreads);
    context->loggingService.Write(MessageType::INFO, "Checking for hyperthreading : %s", isHyperThreadinEnabled ? "enabled":"disabled");
    
    threads = new std::thread[numThreads];

    rowsPerThread = context->height / numThreads;
}


void ThreadedShader::Render(Color * _pixels){

    // TODO

    for (int i = 0; i < numThreads; ++i) {
        int startY = i * rowsPerThread;
        int endY =  startY + rowsPerThread;

        // threads[i] = std::thread([this, startY, endY, _pixels](){
        //     this->ComputeRows(startY, endY, _pixels);
        // });
    }

    //ComputeRows((numThreads-1)*rowsPerThread, context->height, _pixels);

    for (int i = 0; i < numThreads; ++i){
        threads[i].join();
    }

}

bool ThreadedShader::CheckForHyperthreading(){

    FILE *fp;
    char var[5] = {0};
    int corecount = 0;

#ifdef _WIN32

    fp = popen("wmic cpu get NumberOfCores", "r");

    while (fgets(var,sizeof(var),fp) != NULL)
        sscanf(var,"%d",&corecount);

#elif __APPLE__

    fp = popen("sysctl -n hw.physicalcpu", "r");

    while (fgets(var,sizeof(var),fp) != NULL)
        sscanf(var,"%d",&corecount);

#else

    fp = popen("lscpu | grep 'Core(s) per socket' | awk '{print $NF}'", "r");

    fscanf(fd, "%d", &corecount);

#endif

    if(fp)
        pclose(fp);

    return numThreads > corecount;

}

ThreadedShader::~ThreadedShader(){
    delete[] threads;
}