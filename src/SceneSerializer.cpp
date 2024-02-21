#include "SceneSerializer.h"


SceneSerializer::SceneSerializer(RenderingContext * _context){
    this->context = _context;
    this->materialBuilder = new MaterialBuilder(context);
    ResetObject();
}

SceneSerializer::~SceneSerializer(){
    delete materialBuilder;
}

std::vector<std::string>& SceneSerializer::Tokenize(const char * data, const char * delimiter){

    static std::vector<std::string> tokens;

    tokens.clear();

    char * saveptr;

    const char * token =  strtok_r((char*)data, delimiter, &saveptr);

    while( token != NULL ){
        tokens.push_back(std::string(token));
        token = strtok_r(NULL, delimiter, &saveptr);
    }

    return tokens;
}


bool SceneSerializer::CheckBrackets(const char * data, const size_t & size){

    uint32_t invalidBracketCounter = 0 ;

    for(uint32_t pos = 0; pos < size; ++pos){

        if ( data[pos] == '{' ) 
            invalidBracketCounter++;
        

        if ( data[pos] == '}' ){

            if(invalidBracketCounter==0)
                return false;

            invalidBracketCounter--;
        }

    }

    return !invalidBracketCounter;

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

    } else if( tokens[0] == "position" ){

        temp.x = atof(tokens[1].c_str());
        temp.y = atof(tokens[2].c_str());
        temp.z = atof(tokens[3].c_str());
        
        temporaryObject.normal = temp.Normalize();

    } else if( tokens[0] == "scale" ){

        // TODO single or multiple scaling factors

        temp.x = atof(tokens[1].c_str());
        
        if( temporaryObject.type == SpatialType::CUBE  || temporaryObject.type == SpatialType::PLANE){
            temp.y = atof(tokens[2].c_str());
            temp.z = atof(tokens[3].c_str());
        }

        temporaryObject.maxPos = temp;

    }else if( tokens[0] == "material" ){

        for(uint32_t id = 0; id < materials.size(); ++id){

            if( tokens[1] == materials[id].materialname )
                temporaryObject.materialID = materials[id].materialID;
        
        }

    }else if( tokens[0] == "normal" ){

        temp.x = atof(tokens[1].c_str());
        temp.y = atof(tokens[2].c_str());
        temp.z = atof(tokens[3].c_str());
        
        temporaryObject.normal = temp;

    }else if( tokens[0] == "rotation" ){
        // TODO mesh rotation
    }

}

void SceneSerializer::ParseScene(const char * data){

    bool inScene = false;

    const char * delimiter= "\n";
    const char * line  = strtok((char*)data, delimiter);

    while (line != NULL) {

        std::vector<std::string> tokens = Tokenize(line);

        if(inScene){

            if( temporaryObject.type != SpatialType::INVALID ){

                const char * property = CheckProperties(tokens[0].c_str());

                if ( property != NULL){

                    ParseObject(tokens);

                }else{

                    if(tokens[0]=="}"){
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

            if(tokens[0]=="material")
                context->loggingService.Write(MessageType::INFO, "Loading material file %s", tokens[1].c_str());
                // TODO material parsing

            if(tokens[0]=="scene")
                inScene = true;  

        }      

        line = strtok(NULL, delimiter);
    }

}

void SceneSerializer::SaveToFile( const char * _filename){

    file.open(_filename, std::ios::out);

    if( !file.is_open() ){
        fprintf(stderr, "File %s can't be opened or does not exist.\n", _filename);
        exit(-1);
    }

    // TODO

    file.close();

}

void SceneSerializer::LoadFromFile(const char * _filename){

    file.open(_filename, std::ios::in | std::ios::ate);

    if( !file.is_open() ){
        fprintf(stderr, "File %s can't be opened or does not exist.\n", _filename);
        exit(-1);
    }

    size_t fileSize = file.tellg();
    file.seekg( 0, std::ios::beg );

    char * data = new char [fileSize];
    file.read(data, fileSize);
    file.close();

    if( CheckBrackets(data, fileSize) != true ){
        fprintf(stderr, "File %s have invalid format\n", _filename);
        exit(-1);
    }

    ParseScene(data);

    delete [] data;

}