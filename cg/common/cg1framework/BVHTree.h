#pragma once

#include "vector"
#include "Triangle.h"
#include "BoundingBox.h"

class BVHNode;

class BVHTree
{
private:
  BVHNode* root;

  bool intersectWithRay(BVHNode* currentNode, const Triangle*& intersection, float& t, const Ray& ray, float currentDistance, float2& barycentric) const;

public:
  BVHTree(const Triangle* triangles, size_t num_triangles);

  BVHTree(const BVHTree&) = delete;
  BVHTree& operator=(const BVHTree&) = delete;

  bool intersectWithRay(const Triangle*& intersection, float& t, const Ray& ray) const;
  bool intersectWithRay(const Triangle*& intersection, float& t, const Ray& ray, float2& barycentric) const;
};
