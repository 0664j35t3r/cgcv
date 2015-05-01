

#include <limits>

#include "BVHNode.h"

namespace
{
  enum class SortAxis
  {
    X,
    Y,
    Z
  };

  template <SortAxis axis>
  bool sortTriangle(const Triangle& a, const Triangle& b);

  template <>
  bool sortTriangle<SortAxis::X>(const Triangle& a, const Triangle& b)
  {
    float aMin = std::min(a.p1.x, std::min(a.p2.x, a.p3.x));
    float bMin = std::min(b.p1.x, std::min(b.p2.x, b.p3.x));

    return aMin < bMin;
  }

  template <>
  bool sortTriangle<SortAxis::Y>(const Triangle& a, const Triangle& b)
  {
    float aMin = std::min(a.p1.y, std::min(a.p2.y, a.p3.y));
    float bMin = std::min(b.p1.y, std::min(b.p2.y, b.p3.y));

    return aMin < bMin;
  }

  template <>
  bool sortTriangle<SortAxis::Z>(const Triangle& a, const Triangle& b)
  {
    float aMin = std::min(a.p1.z, std::min(a.p2.z, a.p3.z));
    float bMin = std::min(b.p1.z, std::min(b.p2.z, b.p3.z));

    return aMin < bMin;
  }

  void sortTriangles(Triangle* begin, Triangle* end, SortAxis axis)
  {
    switch (axis)
    {
      case SortAxis::X:
        std::sort(begin, end, &sortTriangle<SortAxis::X>);
        break;
      case SortAxis::Y:
        std::sort(begin, end, &sortTriangle<SortAxis::Y>);
        break;
      case SortAxis::Z:
        std::sort(begin, end, &sortTriangle<SortAxis::Z>);
        break;
    }
  }

  SortAxis getLongestAxis(BoundingBox boundingbox)
  {
    float x = boundingbox.width;
    float y = boundingbox.height;
    float z = boundingbox.depth;

    float max = std::max(x, std::max(y, z));

    if (max == x)
      return SortAxis::X;
    if (max == y)
      return SortAxis::Y;
    if (max == z)
      return SortAxis::Z;

    return SortAxis::X;
  }

  BoundingBox calculateBoundingBox(const Triangle* begin, const Triangle* end)
  {
    float maxX = -std::numeric_limits<float>::max();
    float minX = std::numeric_limits<float>::max();
    float maxY = maxX;
    float minY = minX;
    float maxZ = maxX;
    float minZ = minX;

    for (auto t = begin; t != end; ++t)
    {
      float tmpMax;

      tmpMax = std::max(t->p1.x, std::max(t->p2.x, t->p3.x));
      maxX = std::max(tmpMax, maxX);

      tmpMax = std::min(t->p1.x, std::min(t->p2.x, t->p3.x));
      minX = std::min(minX, tmpMax);

      tmpMax = std::max(t->p1.y, std::max(t->p2.y, t->p3.y));
      maxY = std::max(maxY, tmpMax);
      tmpMax = std::min(t->p1.y, std::min(t->p2.y, t->p3.y));
      minY = std::min(minY, tmpMax);

      tmpMax = std::max(t->p1.z, std::max(t->p2.z, t->p3.z));
      maxZ = std::max(maxZ, tmpMax);
      tmpMax = std::min(t->p1.z, std::min(t->p2.z, t->p3.z));
      minZ = std::min(minZ, tmpMax);
    }

    return BoundingBox(minX, minY, minZ, std::fabs(minX - maxX), std::fabs(minY - maxY), std::fabs(minZ - maxZ));
  }
}

BVHNode::BVHNode(Triangle* begin, Triangle* end)
    : boundingBox(calculateBoundingBox(begin, end))
{
  if (end - begin <= 5)
  {
    triangles = std::vector<Triangle>(begin, end);
    left = nullptr;
    right = nullptr;
  }
  else
  {
    // divide
    SortAxis axis = getLongestAxis(boundingBox);
    sortTriangles(begin, end, axis);
    size_t cutPoint = (end - begin) / 2;

    left = new BVHNode(begin, begin + cutPoint);
    right = new BVHNode(begin + cutPoint, end);
  }
}
