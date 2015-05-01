#ifndef INCLUDED_RAY_H
#define INCLUDED_RAY_H

#include "vector.h"

struct Ray
{
  float3 origin;
  float3 direction;

  Ray(const float3& origin, const float3& direction)
      : origin(origin), direction(direction)
  {
  }
};

#endif // INCLUDED_RAY_H
