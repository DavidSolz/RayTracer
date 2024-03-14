#ifndef PLANEBUILDER_H
#define PLANEBUILDER_H

#include "RenderingContext.h"

#define DISK_VERTICES 32

template<SpatialType T>
class ObjectBuilder{};

template<>
class ObjectBuilder < SpatialType::CUBE > {
public:

    static void Build(const Object & cube, RenderingContext * context){

        Vector3 vertices[8] = {
            Vector3(-1, -1, -1),
            Vector3(1, -1, -1),
            Vector3(1, 1, -1),
            Vector3(-1, 1, -1),
            Vector3(-1, 1, 1),
            Vector3(1, 1, 1),
            Vector3(1, -1, 1),
            Vector3(-1, -1, 1),
        };

        for (int32_t i = 0; i < 8; ++i)
            vertices[i] = vertices[i] * cube.maxPos * 0.5f;

        Vector3 faces[] = {
            Vector3(0, 1, 6),
            Vector3(0, 6, 7),
            Vector3(0, 1, 2),
            Vector3(0, 2, 3),
            Vector3(1, 2, 5),
            Vector3(1, 5, 6),

            Vector3(6, 5, 4),
            Vector3(6, 4, 7),

            Vector3(0, 3, 4),
            Vector3(0, 4, 7),
            Vector3(3, 2, 5),
            Vector3(3, 5, 4)
        };

        Object p = {};
        p.type = TRIANGLE;
        p.materialID = cube.materialID;

        for (int32_t f=0; f < 12; ++f){

            Vector3 face = faces[f];

            Vector3 A = vertices[(int32_t)face.x] ;
            Vector3 B = vertices[(int32_t)face.y] ;
            Vector3 C = vertices[(int32_t)face.z] ;

            p.verticeA = cube.position + A;
            p.verticeB = cube.position + B;
            p.verticeC = cube.position + C;

            Vector3 u = (B - A).Normalize();
            Vector3 v = (C - A).Normalize();

            Vector3 normal = Vector3::CrossProduct(u, v).Normalize();

            if (Vector3::DotProduct(normal, (A - cube.position).Normalize() ) < 0.0f)
                normal = normal * -1.0f;

            p.normal = normal;

            context->objects.emplace_back(p);
        }

    }

};

template<>
class ObjectBuilder< SpatialType::DISK > {

public:

    static void Build(const Object & disk, RenderingContext * context){

        const Vector3 reference = Vector3(1, 0, 0);

        const Vector3 U = Vector3::CrossProduct(disk.normal, reference);
        const Vector3 W = Vector3::CrossProduct(disk.normal, U);

        std::vector<Vector3> points;

        points.emplace_back();

        constexpr float delta = 2.0f * M_PI / DISK_VERTICES;

        for(int32_t i = 0; i < DISK_VERTICES; ++i){

            float x = disk.radius * std::cos( delta * i );
            float y = disk.radius * std::sin( delta * i );

            Vector3 point =  U * x + W * y;

            points.emplace_back(point);
        }

        Object p = {};
        p.type = TRIANGLE;
        p.normal = disk.normal;
        p.materialID = disk.materialID;

        for(int32_t i = 1; i <= DISK_VERTICES; ++i){

            p.verticeA = disk.position + points[ 0 ];
            p.verticeB = disk.position + points[ i ];
            p.verticeC = disk.position + points[ i % DISK_VERTICES + 1 ];

            context->objects.emplace_back(p);
        }
    }

};


template<>
class ObjectBuilder< SpatialType::PLANE > {
public:

    static void Build(const Object & plane, RenderingContext * context){

        const Vector3 reference = Vector3(1, 0, 0);

        const Vector3 U = Vector3::CrossProduct(plane.normal, reference).Normalize();
        const Vector3 W = Vector3::CrossProduct(plane.normal, U).Normalize();

        Vector3 vertices[4];

        constexpr float delta = 2.0f * M_PI / 4;
        constexpr float offset = M_PI / 4;

        for(int i=0; i < 4; ++i){

            float angle = delta * i + offset;

            float x = plane.maxPos.x * std::cos(angle);
            float y = plane.maxPos.y * std::sin(angle);

            vertices[i] = plane.position + U * x + W * y;

        }

        Object p = {};

        p.type = TRIANGLE;
        p.normal = plane.normal;
        p.materialID = plane.materialID;

        p.verticeA = vertices[0];
        p.verticeB = vertices[1];
        p.verticeC = vertices[2];

        context->objects.emplace_back(p);

        p.verticeA = vertices[0];
        p.verticeB = vertices[2];
        p.verticeC = vertices[3];

        context->objects.emplace_back(p);

    }

};

#endif