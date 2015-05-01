#include "BVHTree.h"
#include "Triangle.h"
#include "BVHNode.h"
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <iostream>
#include <limits>
#include <math.h>

BVHTree::BVHTree(const Triangle* triangles, size_t num_triangles)
{
  std::vector<Triangle> tris(triangles, triangles + num_triangles);
  this->root = new BVHNode(&tris[0], &tris[0] + num_triangles);
  std::cout << "Triangles: " << num_triangles << std::endl;
}

bool BVHTree::intersectWithRay(BVHNode* currentNode, const Triangle*& intersection, float& t, const Ray& ray, float currentDistance, float2& barycentric) const
{
  if (currentNode->triangles.empty() && (currentNode->left == NULL && currentNode->right == NULL))
    return false;
  bool intersected = false;

  if (!currentNode->triangles.empty())
  {
    // leaf node
    auto it = currentNode->triangles.begin();
    t = std::numeric_limits<float>::max();
    while (it != currentNode->triangles.end())
    {
      float tmpDistance;
      const Triangle* currentTriangle = &(*it);
      float2 bar;
      if (currentTriangle->intersectWithRay(ray, tmpDistance, bar))
      {
        intersected = true;
        if (tmpDistance < t)
        {
          t = tmpDistance;
          intersection = currentTriangle;
          barycentric = bar;
        }
      }
      it++;
    }

    return intersected;
  }
  else
  {
    // test the two possible boxes
    bool intersectLeft;
    bool intersectRight;
    float distanceLeft;
    float distanceRight;
    float3 intPoint;

    intersectLeft = currentNode->left->boundingBox.intersectWithRay(ray, distanceLeft);
    intersectRight = currentNode->right->boundingBox.intersectWithRay(ray, distanceRight);

    if (intersectLeft && !intersectRight)
      return this->intersectWithRay(currentNode->left, intersection, t, ray, currentDistance, barycentric);
    else if (!intersectLeft && intersectRight)
      return this->intersectWithRay(currentNode->right, intersection, t, ray, currentDistance, barycentric);
    else if (intersectLeft && intersectRight)
    {
      const Triangle* leftTriangle;
      const Triangle* rightTriangle;
      float2 barLeft, barRight;
      distanceLeft = std::numeric_limits<float>::max();
      distanceRight = std::numeric_limits<float>::max();

      bool intersectChildLeft = this->intersectWithRay(currentNode->left, leftTriangle, distanceLeft, ray, currentDistance, barLeft);

      bool intersectChildRight = this->intersectWithRay(currentNode->right, rightTriangle, distanceRight, ray, currentDistance, barRight);

      if (intersectChildLeft && distanceLeft <= distanceRight)
      {
        intersection = leftTriangle;
        t = distanceLeft;
        barycentric = barLeft;
        return true;
      }
      else if (intersectChildRight && distanceRight <= distanceLeft)
      {
        intersection = rightTriangle;
        t = distanceRight;
        barycentric = barRight;
        return true;
      }
      else
        return false;

      return false;
    }
    else
      return false;
  }
}

bool BVHTree::intersectWithRay(const Triangle*& intersection, float& t, const Ray& ray) const
{
  float2 barycentric;
  return BVHTree::intersectWithRay(intersection, t, ray, barycentric);
}

bool BVHTree::intersectWithRay(const Triangle*& intersection, float& t, const Ray& ray, float2& barycentric) const
{
  if (this->root->boundingBox.intersectWithRay(ray, t))
  {
    return this->intersectWithRay(this->root, intersection, t, ray, std::numeric_limits<float>::max(), barycentric);
  }
  else
    return false;
}
