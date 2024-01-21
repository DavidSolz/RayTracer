#include "Logger.h"

Logger::Logger(const char * filename){

    uint32_t len = 0;
    while(filename[len++]);

    if (len > 0) 
        output = fopen(filename, "wb");
    
    isSystemStream = false;

    if ( output == nullptr) {
        fprintf(stderr, "Error opening log file: %s", filename);
        isSystemStream = true;
        output =  stdout;
    }

}

Logger::Logger(FILE * file) {

    output = file;
    isSystemStream = true;

}

void Logger::Write(MessageType _type, const char * _data){

    if(_type == MessageType::INFO){
        fprintf(output, "%s\n", _data);
    }else{
        fprintf(output, "[ %s ] : %s\n", _type==MessageType::WARNING?"WARNING":"ERROR", _data);
    }

    fflush(output);
    
}

Logger::~Logger(){

    if( output!= nullptr && !isSystemStream){
        fclose(output);
    }

}