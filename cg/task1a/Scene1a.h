

#ifndef INCLUDED_SCENE_1A
#define INCLUDED_SCENE_1A

#pragma once

#include "Scene.h"

struct Camera
{
  float3 eye;
  float3 lookat;
  float3 up;

  float f;
  float beta;
};

class Scene1a : public Scene
{
private:
  Camera camera;
  float2 viewBox;

  void SetCamera(const float3& eye, const float3& lookat, const float3& up, float f, float beta);

public:
  Scene1a(CgcvConfig::Container& root);

  Scene1a(const Scene1a&) = delete;
  Scene1a& operator=(const Scene1a&) = delete;

  const Camera* getCamera() const { return &camera; }

  float scaleDistance(float d) const;
};

#endif // INCLUDED_SCENE_1A
