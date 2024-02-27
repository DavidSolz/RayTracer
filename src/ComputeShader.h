#ifndef COMPUTESHADER_H
#define COMPUTESHADER_H

#include "IFrameRender.h"
#include "RenderingContext.h"

class ComputeShader : public IFrameRender{
protected:

    RenderingContext * context;

public:

    ComputeShader(RenderingContext * _context);

    virtual void Render(Color * _pixels) = 0;

};

#endif