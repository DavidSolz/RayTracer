#ifndef LOGGER_H
#define LOGGER_H

#include "ILog.h"

#include <stdint.h>
#include <iostream>
#include <fstream>

class Logger : public ILog{
private:

    bool isSystemStream;
    FILE * output;

public:

    Logger(const char * filename);

    Logger(FILE * file);

    void Write(MessageType _type, const char * _data);

    ~Logger();
};

#endif