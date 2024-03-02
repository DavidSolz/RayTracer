#ifndef CONFIGURATOR_H
#define CONFIGURATOR_H

#include "SceneSerializer.h"
#include "ComputeEnvironment.h"
#include "BVHTree.h"

class Configurator{
private:
    
    RenderingContext * context;

    SceneSerializer * serializer;
    
    BVHTree * tree;

    void Initialize();

    void ShowHelp();

public:

    Configurator(RenderingContext * _context);

    void ParseArgs(const size_t & size, char **args);

    ~Configurator();

};



#endif 