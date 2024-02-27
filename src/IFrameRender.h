#ifndef IFRAMERENDER_H
#define IFRAMERENDER_H

#include "RenderingContext.h"

class IFrameRender {
public:
    virtual void Render(Color * _pixels) = 0;
};

#endif

