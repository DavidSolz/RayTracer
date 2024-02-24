#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <fstream>
#include <filesystem>
#include <vector>
#include <cstring>
#include <string>

class Serializer{
protected:
    
    std::vector<std::string>& Tokenize(const std::string & data, const char * delimier = " ") const {
        static std::vector<std::string> tokens;

        tokens.clear();

        char * temp = new char [ data.size( ) + 1];

        memcpy(temp, data.c_str(), data.size());

        temp[data.size()] = '\0';

        const char * token =  strtok(temp, delimier);

        while( token != NULL ){
            tokens.push_back(std::string(token));
            token = strtok(NULL, " ");
        }

        delete[] temp;

        return tokens;
    }

    virtual void Parse(std::ifstream & file, const char * filename) = 0;

public:

    virtual void SaveToFile(const char * _filename) = 0;

    virtual void LoadFromFile(const char * _filename) = 0;

};


#endif