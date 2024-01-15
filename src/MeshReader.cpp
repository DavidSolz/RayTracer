#include "MeshReader.h"

Vector3 MeshReader::ParseVertice(const std::string & line){

    Vector3 temp;

    const char *data = line.c_str()+1;

    std::sscanf(data, "%f %f %f", &temp.x, &temp.y, &temp.z);

    return temp;
}


Vector3 MeshReader::ParseFace(const std::string & line){
    int a,b,c;

    int vertexIndex, textureCoordIndex, normalIndex;

    const char * data = line.c_str();

    int vertexIndex1, textureCoordIndex1, normalIndex1;
    int vertexIndex2, textureCoordIndex2, normalIndex2;
    int vertexIndex3, textureCoordIndex3, normalIndex3;

    sscanf(data, "f %d//%d %d//%d %d//%d",
           &a , &normalIndex1,
           &b , &normalIndex2,
           &c , &normalIndex3);

    return Vector3(a, b, c);
}

Mesh MeshReader::LoadObject(const std::string & filename){

    Mesh mesh = {0};

    std::ifstream input(filename, std::ios::in);

    if (!input.is_open()) {
        fprintf(stderr, "File can't be opened.\n");
        return mesh;
    }

    std::string line;

    while( std::getline(input, line) ){
        if(line.empty())
            continue;

        Vector3 data;

        switch (line[0]){
        case 'v':

            if(line[1]==' '){
                data = ParseVertice(line);
                mesh.vertices.emplace_back(data);
                mesh.numVertices++;
            }

            break;
        
        case 'f':
            
            data = ParseFace(line);
            mesh.indices.emplace_back((int)data.x-1);
            mesh.indices.emplace_back((int)data.y-1);
            mesh.indices.emplace_back((int)data.z-1);
            mesh.numIndices++;

            break;

        default:
            break;
        }
    }

    input.close();

    return mesh;
}