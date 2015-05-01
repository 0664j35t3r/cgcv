
#ifndef RayTracer1a_INCLUDED
#define RayTracer1a_INCLUDED

#pragma once

#include <string>

#include "RayTracer.h"
#include "Scene1a.h"

class RayTracer1a : public RayTracer
{
public:
  RayTracer1a(int width, int height);

  RayTracer1a(const RayTracer1a&) = delete;
  RayTracer1a& operator=(const RayTracer1a&) = delete;

  void render(const Scene1a& scene, const std::string& output);
};

#endif // RayTracer1a_INCLUDED
