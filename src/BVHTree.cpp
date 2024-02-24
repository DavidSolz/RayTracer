#include "BVHTree.h"

BVHTree::BVHTree(){
    this->size = 0;
    this->boxes.clear();

    BoundingBox root;
    root.minimalPosition = Vector3(INFINITY, INFINITY, INFINITY);
    root.maximalPosition = Vector3(-INFINITY, -INFINITY, -INFINITY);
}

void BVHTree::Insert(const Object & _object, const uint32_t & _objectID){

    BoundingBox box = {0};

    box.objectID = _objectID;
    box.leftID = -1;
    box.rightID = -1;

    if( _object.type == SPHERE ){
        Vector3 radiusVector = Vector3(_object.radius, _object.radius, _object.radius);
        box.minimalPosition = Vector3::Minimal(_object.position-radiusVector, _object.position+radiusVector);
        box.maximalPosition = Vector3::Maximal(_object.position-radiusVector, _object.position+radiusVector);

    }else if ( _object.type == CUBE ){

        box.minimalPosition = _object.position;
        box.maximalPosition = _object.maxPos;

    }else if( _object.type == TRIANGLE ){

        Vector3 A = _object.verticeA;
        Vector3 B = _object.verticeB;
        Vector3 C = _object.verticeC;

        Vector3 min = Vector3::Minimal(A, B);
        min = Vector3::Minimal(min, C);

        Vector3 max = Vector3::Maximal(A, B);
        max = Vector3::Maximal(max, C);

        box.minimalPosition = min;
        box.maximalPosition = max;

    }

    // case SpatialType::PLANE:
    //     break;

    // case SpatialType::DISK:
    //     break;

    InsertBox(box);
}

BoundingBox BVHTree::CombineBoxes(const BoundingBox & a, const BoundingBox & b){

    BoundingBox result;

    result.objectID = -1;
    result.leftID = -1;
    result.rightID = -1;

    result.minimalPosition = Vector3::Minimal(a.minimalPosition, b.minimalPosition);
    result.maximalPosition = Vector3::Maximal(a.maximalPosition, b.maximalPosition);
    return result;

}


void BVHTree::Insert(const std::vector<Object> & _objects){

    for(uint32_t id=0; id < _objects.size(); id++){
        Insert(_objects[id], id);
    }

}

void  BVHTree::InsertBox(const BoundingBox & box){

}

uint32_t BVHTree::GetSize() const{
    return size;
}

std::vector<BoundingBox> BVHTree::GetData() const{
    return boxes;
}

BVHTree::~BVHTree(){

}