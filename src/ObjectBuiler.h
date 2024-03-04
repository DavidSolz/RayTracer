#ifndef PLANEBUILDER_H
#define PLANEBUILDER_H

#include "RenderingContext.h"

template<SpatialType T>
class ObjectBuilder{};


template<>
class ObjectBuilder< SpatialType::PLANE > {
private:

    RenderingContext * context;

public:

    ObjectBuilder(RenderingContext * _context){
        this->context = _context;
    }

    void Build(const Object & plane){

        const Vector3 vertices[4] ={
            plane.position,
            plane.position + Vector3(plane.maxPos.x, 0.0f, 0.0f),
            plane.position + Vector3(plane.maxPos.x, 0.0f, plane.maxPos.y),
            plane.position + Vector3(0.0f, 0.0f, plane.maxPos.y)
        };

        Vector3 centroid = ( vertices[0] + vertices[1] + vertices[2] + vertices[3] ) * 0.25f;

        Vector3 desiredNormal = Vector3(0.0f, 1.0f, 1.0f).Normalize();

        Object p;
        p.type = TRIANGLE;
        p.materialID = 0;
        p.normal = desiredNormal;
        
        Vector3 normal = Vector3::CrossProduct(vertices[1] - vertices[2], vertices[3] - vertices[2]).Normalize();
        Vector3 axis = Vector3::CrossProduct(normal, desiredNormal).Normalize();
        float cos = Vector3::DotProduct(normal, desiredNormal);
        float sin = sqrtf( 1.0f - cos*cos );

        Vector3 K[3] = {
            Vector3(0, -axis.z, axis.y),
            Vector3(axis.z, 0, -axis.x),
            Vector3(-axis.y, axis.x, 0)
        };

        Vector3 K2[3] = {
            Vector3(-axis.y * axis.y - axis.z * axis.z, axis.x * axis.y, -axis.x * axis.z),
            Vector3(-axis.x * axis.y, axis.x * axis.x - axis.z * axis.z, axis.y * axis.z),
            Vector3(axis.x * axis.z, axis.y * axis.z, -axis.y * axis.y + axis.x * axis.x)
        };

        Vector3 rot[3] = {
            Vector3(1,  0,  0) + K[0] * sin + K2[0] * (1.0f - cos),
            Vector3(0,  1,  0) + K[1] * sin + K2[1] * (1.0f - cos),
            Vector3(0,  0,  1) + K[2] * sin + K2[2] * (1.0f - cos)
        };

        Vector3 col1 = Vector3(rot[0][0], rot[0][1], rot[0][2]);
        Vector3 col2 = Vector3(rot[1][0], rot[1][1], rot[1][2]);
        Vector3 col3 = Vector3(rot[2][0], rot[2][1], rot[2][2]);

        // First triangle
        
        p.verticeA.x = Vector3::DotProduct( vertices[0] - centroid, col1 );
        p.verticeA.y = Vector3::DotProduct( vertices[0] - centroid, col2 );
        p.verticeA.z = Vector3::DotProduct( vertices[0] - centroid, col3 );

        p.verticeB.x = Vector3::DotProduct( vertices[1] - centroid, col1 );
        p.verticeB.y = Vector3::DotProduct( vertices[1] - centroid, col2 );
        p.verticeB.z = Vector3::DotProduct( vertices[1] - centroid, col3 );

        p.verticeC.x = Vector3::DotProduct( vertices[3] - centroid, col1 );
        p.verticeC.y = Vector3::DotProduct( vertices[3] - centroid, col2 );
        p.verticeC.z = Vector3::DotProduct( vertices[3] - centroid, col3 );

        p.verticeA = p.verticeA + centroid;
        p.verticeB = p.verticeB + centroid;
        p.verticeC = p.verticeC + centroid;

        context->objects.emplace_back(p);

        // Second triangle

        p.verticeA.x = Vector3::DotProduct( vertices[1] - centroid, col1 );
        p.verticeA.y = Vector3::DotProduct( vertices[1] - centroid, col2 );
        p.verticeA.z = Vector3::DotProduct( vertices[1] - centroid, col3 );

        p.verticeB.x = Vector3::DotProduct( vertices[2] - centroid, col1 );
        p.verticeB.y = Vector3::DotProduct( vertices[2] - centroid, col2 );
        p.verticeB.z = Vector3::DotProduct( vertices[2] - centroid, col3 );

        p.verticeC.x = Vector3::DotProduct( vertices[3] - centroid, col1 );
        p.verticeC.y = Vector3::DotProduct( vertices[3] - centroid, col2 );
        p.verticeC.z = Vector3::DotProduct( vertices[3] - centroid, col3 );

        p.verticeA = p.verticeA + centroid;
        p.verticeB = p.verticeB + centroid;
        p.verticeC = p.verticeC + centroid;

        context->objects.emplace_back(p);


    }

};

#endif