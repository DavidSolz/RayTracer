#ifndef BVHTREE_H
#define BVHTREE_H

#include "BoundingBox.h"
#include "RenderingContext.h"

#include <algorithm>
#include <stdio.h>

class BVHTree{
private:

    RenderingContext * context;

    BoundingBox CreateLeaf(const uint32_t & objectID);

    void Insert(const uint32_t & objectID);

    float CalculateArea(const BoundingBox & box);

    int32_t FindBestAxis(const std::vector<int32_t> & ids);

    int32_t Insert(std::vector<int32_t> & ids, const int32_t & parentID = -1, const uint32_t & depth = 0);

    void CheckBalance(const int32_t & currentNode = -1);

    int32_t CalculateDepth(const int32_t & currentNode = -1);

public:
    BVHTree( RenderingContext * _context );

    void BuildBVH();

    uint32_t GetSize() const;

    std::vector<BoundingBox> & GetData() const;

    ~BVHTree();
};




#endif