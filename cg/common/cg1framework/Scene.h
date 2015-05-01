#ifndef SCENE_INCLUDED
#define SCENE_INCLUDED

#pragma once

#include "scene_builder.h"
#include "matrix.h"
#include "formats.h"
#include "Triangle.h"
#include "BVHTree.h"

typedef std::vector<Pointlight> light_list;

class Scene : protected SceneBuilder
{
private:
  float3 background_color;

  std::vector<Triangle> triangles;
  light_list lights;

  BVHTree* bvhTree;

protected:
  void SetCamera(const float3& eye, const float3& lookat, const float3& up, float f, float beta) {}
  void AddTriangle(const float3& p1, const float3& p2, const float3& p3, const float3& n1, const float3& n2, const float3& n3, const float2& t1, const float2& t2, const float2& t3, int materialIndex);
  void AddPointlight(const float3& pos, const float3& color);
  void SetBackground(const float3& background);
  int AddReflectiveMaterial(const float3& diffuse, const float3& specular) { return -1; }
  int AddUntexturedMaterial(const float3& diffuse, const float3& specular, float exp) { return -1; }
  int AddTexturedMaterial(const char* tex, const float3& specular, float exp) { return -1; }

  void buildBVH();

public:
  Scene() = default;

  Scene(const Scene&) = delete;
  Scene& operator=(const Scene&) = delete;

  const light_list& getLightList() const { return lights; }
  float3 background() const { return background_color; }

  bool intersectWithRay(const Triangle*& intersection, float& t, const Ray& ray) const;
  bool intersectWithRay(const Triangle*& intersection, float& t, const Ray& ray, float2& barycentric) const;
};

#endif // SCENE_INCLUDED
