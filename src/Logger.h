#ifndef LOGGER_H
#define LOGGER_H

#include <stdint.h>
#include <iostream>
#include <fstream>
#include <cstdarg>
#include <ctime>

enum MessageType{
    INFO,
    ISSUE,
    WARNING
};

class Logger{
private:

    bool isSystemStream;
    FILE * output;

public:

    Logger();

    Logger(const char * _filename);

    Logger(FILE * _file);

    void BindOutput(const char * _filename);

    void Write(MessageType _type, const char * _format, ...);

    void Write(const char * _data);

    ~Logger();
};

#endif