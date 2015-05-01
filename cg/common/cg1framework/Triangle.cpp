

#include "Triangle.h"

bool Triangle::intersectWithRay(Ray ray, float& t, float2& barycentric) const
{
  static const float epsilon = 0.00001f;

  float3 e1 = p2 - p1;
  float3 e2 = p3 - p1;

  float3 P = cross(ray.direction, e2);

  float det = dot(P, e1);

  if (std::abs(det) < epsilon)
    return false;

  float inv_det = 1.0f / det;

  float3 T = ray.origin - p1;

  barycentric.x = inv_det * dot(P, T);

  if (barycentric.x < 0.0f || barycentric.x > 1.0f)
    return false;

  float3 Q = cross(T, e1);

  barycentric.y = inv_det * dot(Q, ray.direction);

  if (barycentric.y < 0.0f || barycentric.x + barycentric.y > 1.0f)
    return false;

  t = inv_det * dot(Q, e2);

  return t >= 0.0f;
}
