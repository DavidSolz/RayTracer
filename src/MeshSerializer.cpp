#include "MeshSerializer.h"

MeshSerializer::MeshSerializer(RenderingContext * _context, MaterialSerializer * _materalSerializer){
    this->context = _context;
    this->materialSerializer = _materalSerializer;
    this->currentMaterial = 0;
}

int32_t FindAndCopy(char * buffer, const char * source, const char delimiter){

    int32_t position = -1;
    int32_t i = 0;

    while ( source[i] != '\0' ) {

        if (source[i] == delimiter) {
            position = i;
            break;
        }

        buffer[i] = source[i];
        i++;
    }

    buffer[i] = '\0';

    return position;
}

void MeshSerializer::ParseFace(const  std::vector<std::string> & tokens){

    Face face;

    face.materialID = currentMaterial;

    char temp[ BUFFER_SIZE ];
    int32_t pos = 0;
    int32_t offset;

    for(uint32_t id = 1; id < 4; ++id){
        const char * data = tokens[id].c_str();

        offset = FindAndCopy(temp, data, '/');
        face.indices[ id - 1 ] = std::atoi(temp) - 1;

        pos = offset + 1;

        if( data[ pos ] == '/' ){
            FindAndCopy(temp, data+pos+1, '/');
            face.normals[ id - 1 ] = std::atoi(temp) - 1;
            face.uv[id - 1 ] = -1;
            continue;
        }

        offset = FindAndCopy(temp, data + pos, '/');

        if( offset == -1 ){
            face.normals[ id - 1 ] = std::atoi(temp) - 1;
            face.uv[id - 1 ] = -1;
            continue;
        }

        face.uv[id - 1 ] = std::atoi(temp) - 1;

        FindAndCopy(temp, data + pos + offset + 1, '/');
        face.normals[ id - 1 ] = std::atoi(temp) - 1;

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

        temp.vertices[0] = vertices[ indiceA ] * scale + offset;
        temp.vertices[1] = vertices[ indiceB ] * scale + offset;
        temp.vertices[2] = vertices[ indiceC ] * scale + offset;

        temp.position = (temp.vertices[0] + temp.vertices[1] + temp.vertices[2])/3.0f;

        if( faces[id].normals[0] != -1){

            temp.normals[0] = normals[ faces[id].normals[0] ];
            temp.normals[1] = normals[ faces[id].normals[1] ];
            temp.normals[2] = normals[ faces[id].normals[2] ];

        }else{

            Vector3 normal = normals[id];

            temp.normals[0] = normal;
            temp.normals[1] = normal;
            temp.normals[2] = normal;

        }

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


        }else if( tokens[0] == "vn"){

            if(tokens.size() > 3){

                tempVector.x = atof(tokens[1].c_str());
                tempVector.y = atof(tokens[2].c_str());
                tempVector.z = atof(tokens[3].c_str());

                normals.emplace_back(tempVector);

            }else{
                fprintf(stderr, "Invalid normal format.\n");
                return;
            }


        }else if( tokens[0] == "vt"){

            if(tokens.size() > 2){

                tempVector.x = atof(tokens[1].c_str());
                tempVector.y = atof(tokens[2].c_str());

                if(tokens.size() > 3)
                    tempVector.z = atof(tokens[3].c_str());

                uvs.emplace_back(tempVector);

            }else{
                fprintf(stderr, "Invalid texture coordinate format.\n");
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
