#ifndef PLANE_H
#define PLANE_H

#include "SpatialObject.h"

struct Plane : public SpatialObject {
    Vector3 normal;

    Plane(const Vector3& _position, const Vector3& _normal){
        this->normal = _normal;
        this->position = _position;
    }

};



#endif