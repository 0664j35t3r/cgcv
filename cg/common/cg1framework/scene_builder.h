#ifndef SCENE_BUILDER_INCLUDED
#define SCENE_BUILDER_INCLUDED

#pragma once

#include "cfg_container.h"
#include "vector.h"

class SceneBuilder
{
private:
  SceneBuilder(const SceneBuilder&);
  SceneBuilder& operator=(const SceneBuilder&);

public:
  SceneBuilder() {}
  virtual ~SceneBuilder(){};

  virtual void SetCamera(const float3& eye, const float3& lookat, const float3& up, float f, float beta) = 0;
  virtual void AddTriangle(const float3& p1, const float3& p2, const float3& p3, const float3& n1, const float3& n2, const float3& n3, const float2& t1, const float2& t2, const float2& t3, int materialIndex) = 0;
  virtual void AddPointlight(const float3& pos, const float3& color) = 0;
  virtual void SetBackground(const float3& background) = 0;
  virtual int AddReflectiveMaterial(const float3& diffuse, const float3& specular) = 0;
  virtual int AddUntexturedMaterial(const float3& diffuse, const float3& specular, float exp) = 0;
  virtual int AddTexturedMaterial(const char* tex, const float3& specular, float exp) = 0;

  static void Load(SceneBuilder& builder, CgcvConfig::Container& root);
};

#endif // SCENE_BUILDER_INCLUDED
