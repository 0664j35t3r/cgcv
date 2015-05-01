
#ifndef RAYTRACER1b_INCLUDED
#define RAYTRACER1b_INCLUDED

#pragma once

#include <string>
#include <vector>

#include "RayTracer.h"
#include "Scene1b.h"

class RayTracer1b : public RayTracer
{
private:
  surface<R8G8B8A8> normal_img;
  surface<R8G8B8A8> texcoord_img;
  surface<R8G8B8A8> texture_img;
  surface<R8G8B8A8> texture_img_alt;
  std::vector<surface<R8G8B8A8>> phong_img, phong_img_alt;
  std::vector<surface<R8G8B8A8>> shadow_img;

public:
  RayTracer1b(int width, int height);

  RayTracer1b(const RayTracer1b&) = delete;
  RayTracer1b& operator=(const RayTracer1b&) = delete;

  void render(const Scene1b& scene, const ImagePlane& image_plane, int max_bounce, bool output_intermediates);
  void saveIntermediates(const std::string& filename) const;
};

#endif // RAYTRACER1b_INCLUDED
