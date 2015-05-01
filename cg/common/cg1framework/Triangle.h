#pragma once

#include "vector.h"
#include "Ray.h"

struct Triangle
{
  math::float3 p1;
  math::float3 p2;
  math::float3 p3;

  math::float3 normal1;
  math::float3 normal2;
  math::float3 normal3;

  math::float2 texture1;
  math::float2 texture2;
  math::float2 texture3;

  int materialIndex;

  math::float3 triangleNormal;

  Triangle(const math::float3& p1,
           const math::float3& p2,
           const math::float3& p3,
           const math::float3& normal1,
           const math::float3& normal2,
           const math::float3& normal3,
           const math::float2& texture1,
           const math::float2& texture2,
           const math::float2& texture3,
           int materialIndex)
      : p1(p1), p2(p2), p3(p3), normal1(normal1), normal2(normal2), normal3(normal3), texture1(texture1), texture2(texture2), texture3(texture3), triangleNormal(normalize(cross(p2 - p1, p3 - p1))), materialIndex(materialIndex)
  {
  }

  bool intersectWithRay(Ray ray, float& t, float2& barycentric) const;
};
