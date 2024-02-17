#include "MeshReader.h"

Vector3 MeshReader::ParseVertice(const std::string & line){

    char *data = (char*)line.c_str()+2;

    char * tok1 = strtok(data, " ");
    char * tok2 = strtok(NULL, " ");
    char * tok3 = strtok(NULL, " ");

    return (Vector3){std::stof(tok1), std::stof(tok2), std::stof(tok3)};
}

MeshReader::MeshReader(RenderingContext * _context){
    this->context = _context;
}

Vector3 MeshReader::ParseFace(const std::string & line){

    int temp[3];

    char * token_start = (char*)line.c_str()+2;
    char * token_end = nullptr;

    for(int i=0; i<3; i++){

        char * token_end = strpbrk(token_start, " ");
        char * data_str = strtok(token_start, "/");

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

void MeshReader::BuildTriangles(){

    Object temp;
    temp.type = TRIANGLE;
    temp.materialID = 5;

    for(uint32_t i=0; i < context->mesh.numIndices; ++i){

        uint32_t t = 3*i;

        temp.indicesID.x = context->mesh.indices[t];
        temp.indicesID.y = context->mesh.indices[t+1];
        temp.indicesID.z = context->mesh.indices[t+2];
        temp.normal = context->mesh.normals[i];

        context->objects.emplace_back(temp);

    }
}

Mesh * MeshReader::LoadObject(const std::string & filename){

    Mesh * mesh = &context->mesh;

    std::ifstream input(filename, std::ios::in);

    if ( !input.is_open() ) {
        fprintf(stderr, "File can't be opened.\n");
        return NULL;
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
                mesh->vertices.emplace_back(data);
                mesh->numVertices++;
            }

            break;

        case 'f':

            data = ParseFace(line);
            mesh->indices.emplace_back((int)data.x-1);
            mesh->indices.emplace_back((int)data.y-1);
            mesh->indices.emplace_back((int)data.z-1);
            mesh->numIndices++;

            break;

        default:
            break;
        }
    }

    input.close();

    if(mesh->normals.size() == 0)
        mesh->CalculateNormals();

    BuildTriangles();

    return mesh;
}
