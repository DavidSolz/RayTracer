#include "MeshSerializer.h"

MeshSerializer::MeshSerializer(RenderingContext * _context, MaterialSerializer * _materalSerializer){
    this->context = _context;
    this->materialSerializer = _materalSerializer;
    this->currentMaterial = 0;
}

void MeshSerializer::ParseFace(const  std::vector<std::string> & tokens){

    char * data = NULL;
    char * token = NULL;

    Face face;

    face.materialID = currentMaterial;

    for(uint32_t id = 1; id < 4; ++id){

        data = new char[ tokens[id].size() + 1];
        data[ tokens[id].size() ] = '\0';

        memcpy(data, tokens[id].c_str(), tokens[id].size());

        token = strtok(data, "/");

        if (token == NULL) {
            delete[] data;
            continue;
        }

        face.indices[id-1] = atoi(token) - 1;

        token = strtok(NULL, "/");

        if(token != NULL){
            face.texels[id - 1] = atoi(token) - 1;
        }else{
            face.texels[id - 1] = -1;
        }

        token = strtok(NULL, "/");

        if(token != NULL){
            face.normals[id - 1] = atoi(token) - 1;
        }else{
            face.normals[id - 1] = -1;
        }

        delete[] data;
    }
    
    faces.emplace_back(face);

}

void MeshSerializer::CalculateNormals(){

    for(uint32_t id = 0; id < faces.size(); ++id){

        uint32_t indiceA = faces[id].indices[0];
        uint32_t indiceB = faces[id].indices[1];
        uint32_t indiceC = faces[id].indices[2];

        Vector3 A = vertices[ indiceA ];
        Vector3 B = vertices[ indiceB ];
        Vector3 C = vertices[ indiceC ];

        Vector3 u = B - A;
        Vector3 v = C - A;

        Vector3 normal = Vector3::CrossProduct(u, v).Normalize();

        normals.emplace_back(normal);
    }

}

void MeshSerializer::BuildTriangles(){

    const Vector3 offset = Vector3(context->width/2.0f, context->height/2.0f, context->depth/2.0f);

    float scale = 100.0f * context->camera.aspectRatio;

    Object temp;
    temp.type = TRIANGLE;

    for(uint32_t id = 0 ; id < faces.size(); id++){

        uint32_t indiceA = faces[id].indices[0];
        uint32_t indiceB = faces[id].indices[1];
        uint32_t indiceC = faces[id].indices[2];

        temp.verticeA = vertices[ indiceA ] * scale + offset;
        temp.verticeB = vertices[ indiceB ] * scale + offset;
        temp.verticeC = vertices[ indiceC ] * scale + offset;

        temp.position = (temp.verticeA + temp.verticeB + temp.verticeC)/3.0f;

        Vector3 normal = normals[id];

        if( faces[id].normals[0] != -1){

            Vector3 normalA = normals[ faces[id].normals[0] ];
            Vector3 normalB = normals[ faces[id].normals[1] ];
            Vector3 normalC = normals[ faces[id].normals[2] ];

            normal = (normalA + normalB + normalC).Normalize();

        }

        temp.normal = normal;

        temp.materialID = faces[id].materialID;

        context->objects.emplace_back(temp);

    }

}

void MeshSerializer::Parse(std::ifstream & file, const char * filename){

    Vector3 tempVector;
    std::string line;

    while( std::getline(file, line) ){

        if( line.empty() || line[0]=='#')
            continue;

        std::vector<std::string> tokens = Tokenize(line);

        if (tokens.empty()) {
            fprintf(stderr, "Invalid file format: empty line\n");
            continue;
        }

        if( tokens[0] == "v"){

            if(tokens.size() > 3){

                tempVector.x = atof(tokens[1].c_str());
                tempVector.y = atof(tokens[2].c_str());
                tempVector.z = atof(tokens[3].c_str());

                vertices.emplace_back(tempVector);

            }else{
                fprintf(stderr, "Invalid vertice format.\n");
                return;
            }


        }else if( tokens[0] == "f"){

            if(tokens.size() > 3){

                ParseFace(tokens);

            }else{
                fprintf(stderr, "Invalid face format.\n");
                return;
            }

        }else if( tokens[0] == "usemtl" && materialSerializer != NULL){

            if(tokens.size() > 1){

                currentMaterial = materialSerializer->GetMaterial(tokens[1]);

            }else{
                fprintf(stderr, "Invalid usemtl format.\n");
                return;
            }


        }else if( tokens[0] == "vt" ){

            //TODO UV coords

        }

    }

    CalculateNormals();

    BuildTriangles();
    
}

void MeshSerializer::SaveToFile(const char * _filename){
    // TODO
}

void MeshSerializer::LoadFromFile(const char * _filename){

    std::ifstream file(_filename, std::ios::in);

    context->loggingService.Write(MessageType::INFO, "Loading mesh file : %s", _filename);

    if ( !file.is_open() ) {
        fprintf(stderr, "File %s can't be opened.\n", _filename);
        return;
    }

    faces.clear();
    vertices.clear();
    normals.clear();

    Parse(file, _filename);    

    file.close();

}
