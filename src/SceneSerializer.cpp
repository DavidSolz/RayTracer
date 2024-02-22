#include "SceneSerializer.h"


SceneSerializer::SceneSerializer(RenderingContext * _context){
    this->context = _context;
    this->materialReader = new MaterialReader(context);
    ResetObject();
}

SceneSerializer::~SceneSerializer(){
    delete materialReader;
}

std::vector<std::string>& SceneSerializer::Tokenize(const std::string & data){
    static std::vector<std::string> tokens;

    tokens.clear();

    char * temp = new char [ data.size( ) + 1];

    memcpy(temp, data.c_str(), data.size());

    temp[data.size()] = '\0';

    const char * token =  strtok(temp, " ");

    while( token != NULL ){
        tokens.push_back(std::string(token));
        token = strtok(NULL, " ");
    }

    delete[] temp;

    return tokens;
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

bool SceneSerializer::CheckTypes(const char * data){

    char type[20];  
    int result = sscanf(data, "%19s", type);

    if( result != 1)
        return false;

    for(size_t pos = 0; pos < SPATIAL_TYPE_SIZE; ++pos){

        if( strcmp(type, types[pos] ) == 0){
            temporaryObject.type = SpatialType(pos);
            return true;
        }

    }

    return false;

}

void SceneSerializer::ResetObject(){
    temporaryObject = Object();
    temporaryObject.maxPos = Vector3(1.0f, 1.0f, 1.0f);
    temporaryObject.normal = Vector3(0.0f, 1.0f, 0.0f);
    temporaryObject.type = SpatialType::INVALID;
    temporaryObject.materialID = 0;
}

void SceneSerializer::ParseObject(const std::vector<std::string> & tokens){

    Vector3 temp;

    if( tokens[0] == "position" ){

        temp.x = atof(tokens[1].c_str());
        temp.y = atof(tokens[2].c_str());
        temp.z = atof(tokens[3].c_str());
        
        temporaryObject.position = temp;

    } else if( tokens[0] == "radius" ){

        temporaryObject.radius = atof(tokens[1].c_str());

    } else if( tokens[0] == "scale" ){

        temp.x = atof(tokens[1].c_str());
        
        if( temporaryObject.type == SpatialType::CUBE  || temporaryObject.type == SpatialType::PLANE){
            temp.y = atof(tokens[2].c_str());
            temp.z = atof(tokens[3].c_str());
        }

        temporaryObject.maxPos = temp;

    }else if( tokens[0] == "material" ){

        temporaryObject.materialID = materialReader->GetMaterial(tokens[1]); 
        printf("%d\n", temporaryObject.materialID);

    }else if( tokens[0] == "normal" ){

        temp.x = atof(tokens[1].c_str());
        temp.y = atof(tokens[2].c_str());
        temp.z = atof(tokens[3].c_str());
        
        temporaryObject.normal = temp;

    }

}

void SceneSerializer::ParseScene(std::ifstream & file, const char * filename){

    bool inScene = false;

    std::filesystem::path filepath(filename);
    std::filesystem::path directory = filepath.parent_path();
    std::string tempPath;

    std::string line;

    while ( std::getline(file, line, '\n') ) {

        if( line.empty() )
            continue;

        std::vector<std::string> tokens = Tokenize(line);

        if (tokens.empty()) {
            fprintf(stderr, "Invalid file format: empty line\n");
            continue;
        }

        if(inScene){

            if( temporaryObject.type != SpatialType::INVALID ){

                const char * property = CheckProperties(tokens[0].c_str());

                if ( property != NULL){

                    ParseObject(tokens);

                }else{

                    if( tokens[0] == "}" ){
                        temporaryObject.maxPos = temporaryObject.position + temporaryObject.maxPos;
                        context->objects.emplace_back(temporaryObject);
                        ResetObject();
                    }

                }
            

            }else{

                if( CheckTypes( tokens[0].c_str() ) == false ){

                    if(tokens[0]=="}")
                        inScene = false;

                }

            }


        } else{

            if(tokens[0] == "materials"){
                tempPath = directory.string() + "/" + tokens[1];
                //materialReader->LoadFromFile(tempPath.c_str());    
            }else if(tokens[0]=="scene"){
                inScene = true;  
            }

        }      

    }

}

void SceneSerializer::SaveToFile( const char * _filename){

    std::ofstream file(_filename, std::ios::out);

    if( !file.is_open() ){
        fprintf(stderr, "File %s can't be opened or does not exist.\n", _filename);
        exit(-1);
    }

    // TODO

    file.close();

}

void SceneSerializer::LoadFromFile(const char * _filename){

    std::ifstream file(_filename, std::ios::in );

    context->loggingService.Write(MessageType::INFO, "Loading scene file : %s", _filename);

    if( !file.is_open() ){
        fprintf(stderr, "File %s can't be opened or does not exist.\n", _filename);
        exit(-1);
    }

    ParseScene(file, _filename);

    file.close();

}