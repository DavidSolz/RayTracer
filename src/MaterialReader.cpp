#include "MaterialReader.h"


MaterialReader::MaterialReader(RenderingContext * context){
    this->context = context;
    this->builder = new MaterialBuilder(context);
}

std::vector<std::string>& MaterialReader::Tokenize(const std::string & data){
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

uint32_t MaterialReader::GetMaterial(const std::string & _materialName){

    std::map<std::string, uint32_t>::iterator it = gatheredMaterials.find(_materialName);

    if( it != gatheredMaterials.end()){
        return it->second;
    }

    return 0;
}

void MaterialReader::ParseMaterial(std::ifstream & file){

    float temp[3];

    std::string line;
    std::string materialName;

    while ( std::getline(file, line, '\n') ) {

        if( line.empty() )
            continue;

        std::vector<std::string>& tokens = Tokenize(line);

        if (tokens.empty()) {
            fprintf(stderr, "Invalid file format: empty line\n");
            continue;
        }

        if( tokens[0] == "newmtl" ){

            if( !materialName.empty() ){

                if( GetMaterial( materialName ) != 0 ){
                    fprintf(stderr, "Material %s already exists.\n", materialName);
                }else{
                    gatheredMaterials[ materialName ] = builder->Build();
                    context->loggingService.Write(MessageType::INFO, "Registering new material : %s", materialName.c_str());
                }

            }

            materialName = tokens.size()>1 ? tokens[1] : "";

        }else if (tokens[0] == "Ka" || tokens[0] == "Kd") {
            if (tokens.size() >= 4) {
                    temp[0] = atof(tokens[1].c_str());
                    temp[1] = atof(tokens[2].c_str());
                    temp[2] = atof(tokens[3].c_str());

                if (tokens[0] == "Ka") {
                        builder = builder->SetBaseColor(temp[0], temp[1], temp[2]);
                } else {
                        builder = builder->SetTintColor(temp[0], temp[1], temp[2]);
                    }
            } else {
                fprintf(stderr, "Invalid file format\n");
                return;
            }

        } else if (tokens[0] == "Pr") {
            if (tokens.size() >= 2) {
                temp[0] = atof(tokens[1].c_str());
                builder = builder->SetRoughness(temp[0]);
            } else {
                fprintf(stderr, "Invalid file format\n");
                return;
            }

        }else if (tokens[0] == "Ks") {
            //sscanf(data, "%f %f %f", &currentMaterial.specular.R, &currentMaterial.specular.G, &currentMaterial.specular.B);
        } else if (tokens[0] == "Ns") {
            //sscanf(data, "%f", &currentMaterial.specularIntensity);
        } else if (tokens[0] == "d" || tokens[0] == "Tr") {
            if (tokens.size() >= 2) {
                temp[0] = atof(tokens[1].c_str());
                builder = builder->SetTransparency(tokens[0] == "Tr" ? temp[0] : 1.0f - temp[0]);
            } else {
                fprintf(stderr, "Invalid file format\n");
                return;
            }
        }

    }

    if( materialName.empty() )
        return;

    if( GetMaterial(materialName) != 0 ){
        fprintf(stderr, "Material %s already exists.\n", materialName.c_str());
        return;
    }

    gatheredMaterials[ materialName ] = builder->Build();
    context->loggingService.Write(MessageType::INFO, "Registering new material : %s", materialName.c_str());

}

void MaterialReader::SaveToFile( const char * _filename){
    //TODO
}

void MaterialReader::LoadFromFile(const char * _filename){
    std::ifstream file(_filename, std::ios::in);

    context->loggingService.Write(MessageType::INFO, "Loading material file : %s", _filename);

    if( !file.is_open() ){
        fprintf(stderr, "File %s can't be opened or does not exist.\n", _filename);
        exit(-1);
    }

    ParseMaterial(file);

    file.close();

}

MaterialReader::~MaterialReader(){
    delete builder;
}