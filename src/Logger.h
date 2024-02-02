#ifndef LOGGER_H
#define LOGGER_H

#include "ILog.h"

#include <stdint.h>
#include <iostream>
#include <fstream>
#include <ctime>

class Logger : public ILog{
private:

    bool isSystemStream;
    FILE * output;

public:

    Logger();

    Logger(const char * _filename);

    Logger(FILE * _file);

    void BindOutput(const char * _filename);

    void Write(MessageType _type, const char * _data);

    void Write(const char * _data);

    ~Logger();
};

#endif