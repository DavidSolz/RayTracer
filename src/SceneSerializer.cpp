#include "SceneSerializer.h"

SceneSerializer::SceneSerializer(RenderingContext * _context){
    this->context = _context;
    this->materialSerializer = new MaterialSerializer(context);
    this->meshSerializer = new MeshSerializer(_context, materialSerializer);
    ResetObject();
}

SceneSerializer::~SceneSerializer(){

    delete meshSerializer;
    delete materialSerializer;

}

const char * SceneSerializer::CheckProperties(const char * data){

    char propertyType[20];
    int result = sscanf(data, "%19s", propertyType);

    if( result != 1)
        return NULL;

    for(size_t pos = 0; pos < OBJECT_PROPERTIES_SIZE; ++pos){

        if( strcmp(propertyType, properties[pos] ) == 0)
            return properties[pos];


    }

    return NULL;
}

SpatialType SceneSerializer::CheckTypes(const char * data){

    char type[20];
    int result = sscanf(data, "%19s", type);

    if( result != 1)
        return SpatialType::INVALID;

    for(size_t pos = 0; pos < SPATIAL_TYPE_SIZE; ++pos){

        if( strcmp(type, types[pos] ) == 0){
            temporaryObject.type = SpatialType(pos);
            return temporaryObject.type;
        }

    }

    return SpatialType::INVALID;

}

void SceneSerializer::ResetObject(){
    temporaryObject = Object();
    temporaryObject.maxPos = Vector3(1.0f, 1.0f, 1.0f);
    temporaryObject.normal = Vector3(0.0f, 1.0f, 0.0f);
    temporaryObject.type = SpatialType::INVALID;
    temporaryObject.materialID = 0;
}

void SceneSerializer::ParseObject(const std::vector<std::string> & tokens){

    Vector3 temp = {};

    if( tokens[0] == "position" ){

        if(tokens.size() > 3){

            temp.x = atof(tokens[1].c_str());
            temp.y = atof(tokens[2].c_str());
            temp.z = atof(tokens[3].c_str());

            temporaryObject.position = temp;

        }else{
            fprintf(stderr, "Invalid position format\n");
            return;
        }

    } else if( tokens[0] == "radius" ){

        if(tokens.size() > 1){

            temporaryObject.radius = atof(tokens[1].c_str());

        }else{
            fprintf(stderr, "Invalid radius format\n");
            return;
        }

    } else if( tokens[0] == "scale" ){

        if( tokens.size() > 1){

            temp.x = atof(tokens[1].c_str());
            temp.y = temp.x;
            temp.z = temp.z;

            if( tokens.size() > 2 ){

                temp.y = atof(tokens[2].c_str());
                temp.z = temp.y;

                if( tokens.size() > 3){
                    temp.z = atof(tokens[3].c_str());
                }
            }

            temporaryObject.maxPos = temp;
        }else{
            fprintf(stderr, "Invalid scale format\n");
            return;
        }

    }else if( tokens[0] == "material" ){
        if( tokens.size() > 1){

            temporaryObject.materialID = materialSerializer->GetMaterial(tokens[1]);

        }else{
            fprintf(stderr, "Invalid material format\n");
            return;
        }

    }else if( tokens[0] == "normal" ){

        if( tokens.size() > 3){

            temp.x = atof(tokens[1].c_str());
            temp.y = atof(tokens[2].c_str());
            temp.z = atof(tokens[3].c_str());

            temporaryObject.normal = temp;

        }else{
            fprintf(stderr, "Invalid normal format\n");
            return;
        }

    }

}

void SceneSerializer::Parse(std::ifstream & file, const char * filename){

    bool inScene = false;


    std::filesystem::path filepath(filename);
    std::filesystem::path directory = filepath.parent_path();
    std::string tempPath;

    std::string line;

    while ( std::getline(file, line, '\n') ) {

        if( line.empty() )
            continue;

        if( line[0]=='!' )
            fprintf(stdout, "%s\n", line.c_str());

        std::vector<std::string> tokens = Tokenize(line);

        if ( tokens.empty() ) 
            continue;
        

        if(inScene){

            if( temporaryObject.type != SpatialType::INVALID ){

                if ( CheckProperties( tokens[0].c_str() ) != NULL ){

                    ParseObject(tokens);

                }else{

                    if( tokens[0] == "}" ){

                        if( temporaryObject.type == SpatialType::PLANE){
                            ObjectBuilder<PLANE>::Build(temporaryObject, context);
                        }else if( temporaryObject.type == SpatialType::DISK){
                            ObjectBuilder<DISK>::Build(temporaryObject, context);
                        }else if( temporaryObject.type == SpatialType::CUBE){
                            ObjectBuilder<CUBE>::Build(temporaryObject, context);
                        }else{
                            context->objects.emplace_back(temporaryObject);
                        }

                        ResetObject();
                    }

                }


            }else{

                if ( tokens[0] == "mesh"){

                    if(tokens.size() > 1){
                        tempPath = directory.string() + "//" + tokens[1];
                        meshSerializer->LoadFromFile(tempPath.c_str());
                    }else{
                        fprintf(stderr, "Invalid mesh format.\n");
                        return;
                    }

                }else if( CheckTypes( tokens[0].c_str() ) != SpatialType::INVALID ){

                    if(tokens[0]=="}")
                        inScene = false;

                }

            }


        } else{

            if(tokens[0] == "mtllib"){
                tempPath = directory.string() + "/" + tokens[1];
                materialSerializer->LoadFromFile(tempPath.c_str());
            }else if(tokens[0]=="scene"){
                inScene = true;
            }

        }

    }

}

void SceneSerializer::LoadFromFile(const char * _filename){

    std::ifstream file(_filename, std::ios::in);

    context->loggingService.Write(MessageType::INFO, "Loading scene file : %s", _filename);

    if( !file.is_open() ){
        fprintf(stderr, "File %s can't be opened or does not exist.\n", _filename);
        exit(-1);
    }

    Parse(file, _filename);

    file.close();

}
