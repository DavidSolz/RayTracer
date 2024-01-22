#ifndef BVHTREE_H
#define BVHTREE_H

#include "BoundingBox.h"
#include "Object.h"
#include "Mesh.h"


class BVHTree{
private:
    
    uint32_t size;
    std::vector<BoundingBox> boxes;

    void InsertBox(const BoundingBox & box);

public:
    BVHTree();

    void Insert(const Object & _object, const uint32_t & _objectID, const Mesh * mesh);

    void Insert(const std::vector<Object> & _objects, const Mesh * mesh);

    uint32_t GetSize() const;

    std::vector<BoundingBox> GetData() const;

    ~BVHTree();
};




#endif