#include "MeshReader.h"

Vector3 MeshReader::ParseVertice(const std::string & line){

    char *data = (char*)line.c_str()+2;

    char * tok1 = strtok(data, " ");
    char * tok2 = strtok(NULL, " ");
    char * tok3 = strtok(NULL, " ");

    return (Vector3){std::stof(tok1), std::stof(tok2), std::stof(tok3)};
}


Vector3 MeshReader::ParseFace(const std::string & line){

    int temp[3];

    char * token_start = (char*)line.c_str()+2;
    char * token_end = nullptr;

    const char * delimiter = "/";

    for(int i=0; i<3; i++){

        char * token_end = strpbrk(token_start, " ");
        char * data_str = strtok(token_start, delimiter);

        temp[i] = atoi(data_str);
        // char * texture = strtok(NULL, delimiter);
        // char * normal = strtok(NULL, delimiter);

        token_start =  token_end;

        if(token_start == nullptr)
            break;

        token_start++;
    }

    return Vector3(temp[0], temp[1], temp[2]);
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
