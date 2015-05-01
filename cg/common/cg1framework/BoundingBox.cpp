

#include "BoundingBox.h"

bool BoundingBox::intersectWithRay(Ray ray, float& t) const
{
  float tmin, tmax, tymin, tymax, tzmin, tzmax;

  int sign[3];

  float t0 = 0;
  float t1 = std::numeric_limits<float>::max();

  float3 leftCorner;
  float3 rightBottomCorner;
  leftCorner.x = x;
  leftCorner.y = y;
  leftCorner.z = z;

  rightBottomCorner.x = x + this->width;
  rightBottomCorner.y = y + this->height;
  rightBottomCorner.z = z + this->depth;

  float3 inverseDirection;
  inverseDirection.x = 1 / ray.direction.x;
  inverseDirection.y = 1 / ray.direction.y;
  inverseDirection.z = 1 / ray.direction.z;

  sign[0] = inverseDirection.x < 0;
  sign[1] = inverseDirection.y < 0;
  sign[2] = inverseDirection.z < 0;

  float3 parameters[2];
  parameters[0] = leftCorner;
  parameters[1] = rightBottomCorner;

  tmin = (parameters[sign[0]].x - ray.origin.x) * inverseDirection.x;
  tmax = (parameters[1 - sign[0]].x - ray.origin.x) * inverseDirection.x;
  tymin = (parameters[sign[1]].y - ray.origin.y) * inverseDirection.y;
  tymax = (parameters[1 - sign[1]].y - ray.origin.y) * inverseDirection.y;

  if ((tmin > tymax) || (tymin > tmax))
    return false;
  if (tymin > tmin)
    tmin = tymin;
  if (tymax < tmax)
    tmax = tymax;
  tzmin = (parameters[sign[2]].z - ray.origin.z) * inverseDirection.z;
  tzmax = (parameters[1 - sign[2]].z - ray.origin.z) * inverseDirection.z;
  if ((tmin > tzmax) || (tzmin > tmax))
    return false;
  if (tzmin > tmin)
    tmin = tzmin;
  if (tzmax < tmax)
    tmax = tzmax;

  t = tmin;

  if (tmin < 0)
    t = tmax;

  return tmax > t0;
}
