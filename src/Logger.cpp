#include "Logger.h"

Logger::Logger(const char * _filename){

    uint32_t len = 0;
    while( _filename[len++] );

    if (len > 0) 
       BindOutput(_filename);
    
    if ( output == nullptr) {
        fprintf(stderr, "Error opening log file: %s", _filename);
        isSystemStream = true;
        output =  stdout;
    }

}

Logger::Logger(FILE * _file) {

    output = _file;
    isSystemStream = true;

}

Logger::Logger(){
    output = stdout;
    isSystemStream = true;
}

void Logger::BindOutput(const char * _filename){

    if( output != nullptr && !isSystemStream )
        fclose(output);

    output = fopen(_filename, "wb");
    isSystemStream = false;

}

void Logger::Write(MessageType _type, const char * _data){

    static const char *types[]= {
        "INFO",
        "WARNING",
        "ERROR"
    };

    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);

    std::fprintf(output, "[%04d-%02d-%02d %02d:%02d:%02d] [%s] : %s\n",
                localTime->tm_year + 1900, localTime->tm_mon + 1, localTime->tm_mday,
                localTime->tm_hour, localTime->tm_min, localTime->tm_sec,
                types[ _type ],
                _data);
    fflush(output);
    
}

void Logger::Write(const char * _data){


    std::fprintf(output, "%s\n", _data);
    fflush(output);
    
}

Logger::~Logger(){

    if( output!= nullptr && !isSystemStream){
        fclose(output);
    }

}