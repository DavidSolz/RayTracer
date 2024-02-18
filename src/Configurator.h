#ifndef CONFIGURATOR_H
#define CONFIGURATOR_H

#include "RenderingContext.h"

class Configurator{
private:
    
    RenderingContext * context;

    void ShowHelp();

public:

    Configurator(RenderingContext * _context);

    void ParseArgs(const size_t & size, char **args);

};



#endif 