#ifndef RAYTRACER_INCLUDED
#define RAYTRACER_INCLUDED

#pragma once

#include <string>
#include "surface.h"

class RayTracer
{
protected:
  surface<R8G8B8A8> output_image;

public:
  RayTracer(int width, int height);

  RayTracer(const RayTracer&) = delete;
  RayTracer& operator=(const RayTracer&) = delete;

  void SaveOutput(const std::string& filename);
  void SaveOutput(const std::string& filename, int upsample);
};

#endif
