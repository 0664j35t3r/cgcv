#pragma once

#include <vector>

#include "Triangle.h"
#include "BoundingBox.h"

class BVHNode
{
public:
  BVHNode* left;
  BVHNode* right;
  BoundingBox boundingBox;

  std::vector<Triangle> triangles;

  BVHNode(Triangle* begin, Triangle* end);
};
