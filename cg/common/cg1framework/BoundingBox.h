#pragma once

#include <limits>

#include "Ray.h"

struct BoundingBox
{
  float x;
  float y;
  float z;
  float width;
  float height;
  float depth;

  BoundingBox(float x, float y, float z, float width, float height, float depth)
      : x(x), y(y), z(z), width(width), height(height), depth(depth)
  {
  }

  bool intersectWithRay(Ray ray, float& t) const;
};
