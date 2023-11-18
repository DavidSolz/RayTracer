#ifndef IFRAMERENDER_H
#define IFRAMERENDER_H

#include "RenderingContext.h"

class IFrameRender {
public:
    virtual void Init(RenderingContext * _context) = 0;
    virtual void Render(Color * _array) = 0;
};

#endif

