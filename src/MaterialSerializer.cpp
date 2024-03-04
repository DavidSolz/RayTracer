#include "MaterialSerializer.h"


MaterialSerializer::MaterialSerializer(RenderingContext * context){
    this->context = context;
    this->builder = new MaterialBuilder(context);
}

uint32_t MaterialSerializer::GetMaterial(const std::string & _materialName){

    std::map<std::string, uint32_t>::iterator it = gatheredMaterials.find(_materialName);

    if( it != gatheredMaterials.end()){
        return it->second;
    }

    return DEFAULT_MATERIAL_ID;
}

void MaterialSerializer::Parse(std::ifstream & file, const char * filename){

    float temp[3];

    std::filesystem::path filepath(filename);
    std::filesystem::path directory = filepath.parent_path();
    std::string tempPath;

    std::string line;
    std::string materialName;

    while ( std::getline(file, line, '\n') ) {

        if( line.empty()  || line[0] == '#')
            continue;

        std::vector<std::string>& tokens = Tokenize(line);

        if (tokens.empty()) {
            fprintf(stderr, "Invalid file format: empty line\n");
            continue;
        }

        if( tokens[0] == "newmtl" ){

            if( !materialName.empty() ){

                if( GetMaterial( materialName ) != DEFAULT_MATERIAL_ID ){
                    fprintf(stderr, "Material %s already exists.\n", materialName.c_str());
                }else{
                    gatheredMaterials[ materialName ] = builder->Build();
                    context->loggingService.Write(MessageType::INFO, "Registering new material : %s", materialName.c_str());
                }

                materialName = "";
            }

            if( tokens.size() > 1)
                materialName = tokens[1];

        }else if (tokens[0] == "Ka") {
            if (tokens.size() > 3) {

                temp[0] = atof(tokens[1].c_str());
                temp[1] = atof(tokens[2].c_str());
                temp[2] = atof(tokens[3].c_str());


                builder = builder->SetBaseColor(temp[0], temp[1], temp[2]);

            } else {
                fprintf(stderr, "Invalid ambient color format\n");
                return;
            }

        }else if (tokens[0] == "Kd"){
            if (tokens.size() > 3) {

                temp[0] = atof(tokens[1].c_str());
                temp[1] = atof(tokens[2].c_str());
                temp[2] = atof(tokens[3].c_str());

                builder = builder->SetTintColor(temp[0], temp[1], temp[2]);

            } else {
                fprintf(stderr, "Invalid tint color format\n");
                return;
            }
        }else if (tokens[0] == "Pr") {

            if (tokens.size() > 1) {
                temp[0] = atof(tokens[1].c_str());
                builder = builder->SetRoughness(temp[0]);
            } else {
                fprintf(stderr, "Invalid roughness format\n");
                return;
            }

        }else if (tokens[0] == "Pm") {

            if (tokens.size() > 1) {
                temp[0] = atof(tokens[1].c_str());
                builder = builder->SetSmoothness(temp[0]);
            } else {
                fprintf(stderr, "Invalid metallic format\n");
                return;
            }

        }else if (tokens[0] == "Ps") {

            if (tokens.size() > 1) {
                temp[0] = atof(tokens[1].c_str());
                builder = builder->SetSheen(temp[0]);
            } else {
                fprintf(stderr, "Invalid sheen format\n");
                return;
            }

        }else if (tokens[0] == "Pc") {

            if (tokens.size() > 1) {
                temp[0] = atof(tokens[1].c_str());
                builder = builder->SetClearcoatThickness(temp[0]);
            } else {
                fprintf(stderr, "Invalid clearcoat thickness format\n");
                return;
            }

        }else if (tokens[0] == "Pcr") {

            if (tokens.size() > 1) {
                temp[0] = atof(tokens[1].c_str());
                builder = builder->SetClearcoatRoughness(temp[0]);
            } else {
                fprintf(stderr, "Invalid clearcoat roughness format\n");
                return;
            }

        }else if (tokens[0] == "Ke") {

            if (tokens.size() > 1) {
                temp[0] = atof(tokens[1].c_str());
                builder = builder->SetEmission(temp[0]);
            } else {
                fprintf(stderr, "Invalid emission format\n");
                return;
            }

        }else if (tokens[0] == "aniso") {

            if (tokens.size() > 1) {
                temp[0] = atof(tokens[1].c_str());
                builder = builder->SetAnisotropy(temp[0]);
            } else {
                fprintf(stderr, "Invalid anisotropy format\n");
                return;
            }

        }else if (tokens[0] == "anisor") {

            if (tokens.size() > 1) {
                temp[0] = atof(tokens[1].c_str());
                builder = builder->SetAnisotropyRotation(temp[0]);
            } else {
                fprintf(stderr, "Invalid anisotropy rotation format\n");
                return;
            }

        }else if (tokens[0] == "Ni") {
            if (tokens.size() > 1) {
                temp[0] = atof(tokens[1].c_str());
                builder = builder->SetRefractiveIndex(temp[0]);
            } else {
                fprintf(stderr, "Invalid index of refraction format\n");
                return;
            }

        }else if (tokens[0] == "Ks") {
            if (tokens.size() > 3) {
                    temp[0] = atof(tokens[1].c_str());
                    temp[1] = atof(tokens[2].c_str());
                    temp[2] = atof(tokens[3].c_str());

                builder = builder->SetSpecularColor(temp[0], temp[1], temp[2]);
            } else {
                fprintf(stderr, "Invalid specular color format\n");
                return;
            }
        }else if (tokens[0] == "Tf") {
            if (tokens.size() > 3) {
                    temp[0] = atof(tokens[1].c_str());
                    temp[1] = atof(tokens[2].c_str());
                    temp[2] = atof(tokens[3].c_str());

                builder = builder->SetTransmissionFilter(temp[0], temp[1], temp[2]);
            } else {
                fprintf(stderr, "Invalid transmission filter format\n");
                return;
            }
        } else if (tokens[0] == "Ns") {
            if (tokens.size() > 1) {
                temp[0] = atof(tokens[1].c_str());
                builder = builder->SetSpecularIntensity(temp[0]);
            } else {
                fprintf(stderr, "Invalid specular intensity format\n");
                return;
            }
        } else if (tokens[0] == "d" || tokens[0] == "Tr") {
            if (tokens.size() > 1) {
                temp[0] = atof(tokens[1].c_str());
                builder = builder->SetTransparency(tokens[0] == "Tr" ? temp[0] : 1.0f - temp[0]);
            } else {
                fprintf(stderr, "Invalid opacity/transparency format\n");
                return;
            }
        } else if (tokens[0] == "map_Ka"){
            if (tokens.size() > 1) {

                tempPath = directory.string() + "/" + tokens[1];
                builder = builder->AttachTexture(tempPath.c_str());

            } else {
                fprintf(stderr, "Invalid texture format\n");
                return;
            }
        }

    }

    if( materialName.empty() )
        return;

    if( GetMaterial(materialName) != DEFAULT_MATERIAL_ID ){
        fprintf(stderr, "Material %s already exists.\n", materialName.c_str());
        return;
    }

    gatheredMaterials[ materialName ] = builder->Build();
    context->loggingService.Write(MessageType::INFO, "Registering new material : %s", materialName.c_str());

}

void MaterialSerializer::LoadFromFile(const char * _filename){
    std::ifstream file(_filename, std::ios::in);

    context->loggingService.Write(MessageType::INFO, "Loading material file : %s", _filename);

    if( !file.is_open() ){
        fprintf(stderr, "File %s can't be opened or does not exist.\n", _filename);
        exit(-1);
    }

    Parse(file, _filename);

    file.close();

}

MaterialSerializer::~MaterialSerializer(){
    delete builder;
}
