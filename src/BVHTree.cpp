#include "BVHTree.h"

BVHTree::BVHTree(RenderingContext * _context){
    this->context = _context;
    context->boxes.clear();
}

BoundingBox BVHTree::CreateLeaf(const uint32_t & objectID){

    BoundingBox box = {0};
    box.minimalPosition = Vector3(INFINITY, INFINITY, INFINITY);
    box.maximalPosition = Vector3(-INFINITY, -INFINITY, -INFINITY);
    box.objectID = objectID;
    box.leftID = -1;
    box.rightID = -1;

    Object & object = context->objects[objectID];

    Vector3 radiusVector = Vector3(object.radius, object.radius, object.radius);

    if( object.type == SPHERE){

        box.minimalPosition = object.position-radiusVector;
        box.maximalPosition = object.position+radiusVector;

    }else if ( object.type == CUBE ){

        box.minimalPosition = Vector3::Minimal(object.position, object.maxPos);
        box.maximalPosition = Vector3::Maximal(object.position, object.maxPos);

    }else if( object.type == TRIANGLE ){

        Vector3 A = object.verticeA;
        Vector3 B = object.verticeB;
        Vector3 C = object.verticeC;

        Vector3 min = Vector3::Minimal(A, B);
        min = Vector3::Minimal(min, C);

        Vector3 max = Vector3::Maximal(A, B);
        max = Vector3::Maximal(max, C);

        box.minimalPosition = min;
        box.maximalPosition = max;

    }else if( (object.type == DISK) || (object.type == PLANE) ){

        // TODO

        box.minimalPosition = object.position - radiusVector;
        box.maximalPosition = object.position + radiusVector;

    }

    return box;
}

void BVHTree::BuildBVH(){

    std::vector<int32_t> ids;


    for(int32_t id = 0; id < context->objects.size(); ++id){
        ids.emplace_back(id);
    }

    Insert(ids, 0);
    //CheckBalance(0);
}

int32_t BVHTree::CalculateDepth(const int32_t & currentNode){
    if( currentNode == -1 )
        return 0;

    int32_t sizeLeft = CalculateDepth( context->boxes[currentNode].leftID );
    int32_t sizeRight = CalculateDepth( context->boxes[currentNode].rightID );

    return 1 + sizeLeft + sizeRight;
}

void BVHTree::CheckBalance(const int32_t & currentNode){
    if(currentNode == -1)
        return;

    CheckBalance( context->boxes[currentNode].leftID );
    CheckBalance( context->boxes[currentNode].rightID );

    int32_t leftChild = context->boxes[currentNode].leftID;
    int32_t rightChild = context->boxes[currentNode].rightID;

    int32_t sizeLeft = CalculateDepth(leftChild);
    int32_t sizeRight = CalculateDepth(rightChild);

    int32_t threshold = 2;

    if( abs(sizeLeft - sizeRight) > threshold){
        fprintf(stderr, "Unbalanced branch.\n");
    }

}

float BVHTree::CalculateArea(const BoundingBox & box){

    Vector3 edgesLen = box.maximalPosition - box.minimalPosition;

    float facesA = 2.0f * edgesLen.x * edgesLen.y;
    float facesB = 2.0f * edgesLen.x * edgesLen.z;
    float facesC = 2.0f * edgesLen.y * edgesLen.z;

    return facesA + facesB + facesC;
}


int32_t BVHTree::FindBestAxis(const std::vector<int32_t> & ids){

    float bestCost = INFINITY;
    int32_t bestSplit = -1;

    BoundingBox bestLeft, bestRight;
    BoundingBox currentLeft, currentRight;

    currentLeft.maximalPosition = Vector3(-INFINITY, -INFINITY, -INFINITY);
    currentRight.maximalPosition = Vector3(-INFINITY, -INFINITY, -INFINITY);
    currentLeft.minimalPosition = Vector3(INFINITY, INFINITY, INFINITY);
    currentRight.minimalPosition = Vector3(INFINITY, INFINITY, INFINITY);

    for (int32_t i = 0; i < ids.size(); ++i) {

        BoundingBox box = CreateLeaf(ids[i]);

        currentRight.minimalPosition = Vector3::Minimal(currentRight.minimalPosition, box.minimalPosition);
        currentRight.maximalPosition = Vector3::Maximal(currentRight.maximalPosition, box.maximalPosition);
    }

    for (int32_t id = 1; id < ids.size(); ++id) {

        BoundingBox box = CreateLeaf(ids[id]);

        currentLeft.minimalPosition = Vector3::Minimal(currentLeft.minimalPosition, box.minimalPosition);
        currentLeft.maximalPosition = Vector3::Maximal(currentLeft.maximalPosition, box.maximalPosition);
        currentRight.minimalPosition = Vector3::Minimal(currentRight.minimalPosition, box.minimalPosition);
        currentRight.maximalPosition = Vector3::Maximal(currentRight.maximalPosition, box.maximalPosition);

        float splitCost = CalculateArea(currentLeft) * id + CalculateArea(currentRight) * (ids.size() - id);

        if (splitCost < bestCost) {
            bestCost = splitCost;
            bestSplit = id;
            bestLeft = currentLeft;
            bestRight = currentRight;
        }
    }

    return bestSplit;
}

int32_t BVHTree::Insert(std::vector<int32_t> & ids, const int32_t & parentID, const uint32_t & depth){

    if( ids.empty() )
        return -1;

    if( ids.size() == 1 ){
        BoundingBox box = CreateLeaf( ids[0] );
        int32_t boxID = context->boxes.size();
        context->boxes.emplace_back(box);
        return boxID;
    }

    int32_t currentNodeID = context->boxes.size();

    int32_t splitAxis = depth%3;

    std::vector<Object> & objects = context->objects;

    std::sort(ids.begin(), ids.end(),
        [splitAxis, &objects](const int32_t & a, const int32_t & b){
            return objects[a].position[splitAxis] < objects[b].position[splitAxis];
        }
    );

    int32_t bestSplit = ids.size() >> 1;

    std::vector<int32_t> left(ids.begin(), ids.begin() + bestSplit);
    std::vector<int32_t> right(ids.begin() + bestSplit, ids.end());

    BoundingBox temp;
    temp.minimalPosition = Vector3(INFINITY, INFINITY, INFINITY);
    temp.maximalPosition = Vector3(-INFINITY, -INFINITY, -INFINITY);
    temp.leftID = -1;
    temp.rightID = -1;
    temp.objectID = -1;

    context->boxes.emplace_back(temp);

    int32_t leftChildID = Insert(left, currentNodeID, depth + 1);
    int32_t rightChildID = Insert(right, currentNodeID, depth + 1);

    if( leftChildID > 0)
        context->boxes[currentNodeID].minimalPosition = Vector3::Minimal(context->boxes[leftChildID].minimalPosition, context->boxes[rightChildID].minimalPosition);

    if( rightChildID > 0)
        context->boxes[currentNodeID].maximalPosition = Vector3::Maximal(context->boxes[leftChildID].maximalPosition, context->boxes[rightChildID].maximalPosition);

    context->boxes[currentNodeID].rightID = rightChildID;
    context->boxes[currentNodeID].leftID  = leftChildID;

    return currentNodeID;
}

uint32_t BVHTree::GetSize() const{
    return context->boxes.size();
}

std::vector<BoundingBox> & BVHTree::GetData() const{
    return context->boxes;
}

BVHTree::~BVHTree(){

}
