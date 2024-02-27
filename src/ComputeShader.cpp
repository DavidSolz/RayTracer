#include "ComputeShader.h"


ComputeShader::ComputeShader(RenderingContext * _context){
    this->context = _context;
    context->loggingService.Write(MessageType::INFO, "Building new shader");
}