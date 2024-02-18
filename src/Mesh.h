#ifndef MESH_H
#define MESH_H

#include <stdint.h>
#include <vector>

#include "Object.h"

struct Mesh{

    uint32_t numVertices;
    uint32_t numIndices;

    std::vector<Vector3> vertices;
    std::vector<Vector3> normals;
    std::vector<int> indices;

    void Translate(const Vector3 & offset, const float & scale, const float & angle){
        for(uint32_t i=0; i < numVertices; ++i){

            Vector3 temp = vertices[i];

            vertices[i].x = temp.x * cos(angle) + temp.z * sin(angle);
            vertices[i].z = -temp.x * sin(angle) + temp.z * cos(angle);

            vertices[i] = vertices[i] * scale + offset;
        }
        CalculateNormals();
    }

    void CalculateNormals(){

        if(normals.size()>0)
            return;

        normals.clear();

        for(uint32_t i=0; i < numIndices; ++i){

            uint32_t IDX = 3*i;

            uint32_t IDA = indices[IDX];
            uint32_t IDB = indices[IDX+1];
            uint32_t IDC = indices[IDX+2];

            Vector3 A = vertices[ IDA ];
            Vector3 B = vertices[ IDB ];
            Vector3 C = vertices[ IDC ];

            Vector3 u = B - A;
            Vector3 v = C - A;

            Vector3 normal = Vector3::CrossProduct(u,v).Normalize();

            normals.emplace_back(normal);
        }

    }

};

#endif

