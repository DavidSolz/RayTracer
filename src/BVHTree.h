#ifndef BVHTREE_H
#define BVHTREE_H

#include "BoundingBox.h"
#include "Object.h"

#include <vector>

class BVHTree{
private:
    
    uint32_t size;
    std::vector<BoundingBox> boxes;

    BoundingBox CombineBoxes(const BoundingBox & a, const BoundingBox & b);

    void InsertBox(const BoundingBox & box);

public:
    BVHTree();

    void Insert(const Object & _object, const uint32_t & _objectID);

    void Insert(const std::vector<Object> & _objects);

    uint32_t GetSize() const;

    std::vector<BoundingBox> GetData() const;

    ~BVHTree();
};




#endif