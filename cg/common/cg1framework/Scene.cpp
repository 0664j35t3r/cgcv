

#include "config_utils.h"
#include "Scene.h"
#include "../cgmath.h"

using namespace CgcvConfig;

void Scene::AddTriangle(const float3& p1, const float3& p2, const float3& p3, const float3& n1, const float3& n2, const float3& n3, const float2& t1, const float2& t2, const float2& t3, int materialIndex)
{
  triangles.emplace_back(p1, p2, p3, n1, n2, n3, t1, t2, t3, materialIndex);
}

void Scene::AddPointlight(const float3& pos, const float3& color)
{
  lights.push_back(Pointlight(pos, color));
}

void Scene::SetBackground(const float3& background)
{
  background_color = background;
}

void Scene::buildBVH()
{
  this->bvhTree = new BVHTree(&triangles[0], triangles.size());
}

bool Scene::intersectWithRay(const Triangle*& intersection, float& t, const Ray& ray) const
{
  return bvhTree->intersectWithRay(intersection, t, ray);
}

bool Scene::intersectWithRay(const Triangle*& intersection, float& t, const Ray& ray, float2& barycentric) const
{
  return bvhTree->intersectWithRay(intersection, t, ray, barycentric);
}
