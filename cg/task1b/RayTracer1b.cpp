#include <iostream>
#include <iomanip>
#include <sys/stat.h>
#include <sys/types.h>
#include <sstream>
#include "config_utils.h"
#include "RayTracer1b.h"
#include "Scene1b.h"

#ifdef _WIN32
#include <direct.h>
#endif

const char* taskname = "cgtask1b";
const char* version_string = "CG/task1b framework version 1.2";

void run_testcase(CgcvConfig::Testcase* testcase, const std::string& output, int width, int height)
{
  std::cout << "setting up scene..." << std::endl;

  int max_bounce = getAttribute<int>(*testcase, "max_bounce", 0);
  bool output_intermediates = getAttribute<bool>(*testcase, "output_intermediates", false);

  ImagePlane imagePlane;
  if (testcase->getNumElements("image_plane"))
  {
    CgcvConfig::Container* image_plane_cfg = testcase->getContainer("image_plane");
    imagePlane.a = getVector(*image_plane_cfg, "a", float3(-1.0f, -1.0f, 0.0f));
    imagePlane.b = getVector(*image_plane_cfg, "b", float3(0.0f, 0.0f, 1.0f));
    imagePlane.c = getVector(*image_plane_cfg, "c", float3(1.0f, 0.0f, 0.0f));
    imagePlane.d = getVector(*image_plane_cfg, "d", float3(0.0f, -1.0f, 0.0f));
  }
  else
  {
    std::cout << "ERROR: no image plane defined\n" << std::endl;
    return;
  }

  Scene1b scene(*testcase);

  RayTracer1b rayTracer(width, height);

  std::cout << "rendering..." << std::endl;

  rayTracer.render(scene, imagePlane, max_bounce, output_intermediates);

  std::cout << "saving output to '" << output << "'..." << std::endl;
  std::stringstream dir;
  dir << "output/" << output;
#ifdef _WIN32
  _mkdir(dir.str().c_str());
#else
  mkdir(dir.str().c_str(), 0777);
#endif
  rayTracer.SaveOutput(dir.str() + "/" + output);
  if (output_intermediates)
    rayTracer.saveIntermediates(dir.str() + "/" + output);

  std::cout << "done!" << std::endl;
}

RayTracer1b::RayTracer1b(int width, int height) : RayTracer(width, height)
{
}

namespace
{
  void saveImageVector(const std::vector<surface<R8G8B8A8>>& images, const std::string& filename, const char* name)
  {
    for (int i = 0; i < images.size(); ++i)
    {
      std::ostringstream fn;
      fn << filename << '_' << name << '_' << i << ".png";
      images[i].save(fn.str().c_str());
    }
  }
}

void RayTracer1b::saveIntermediates(const std::string& filename) const
{
  normal_img.save((filename + "_normal.png").c_str());
  texcoord_img.save((filename + "_texcoord.png").c_str());
  texture_img.save((filename + "_diffuse.png").c_str());
  texture_img_alt.save((filename + "_diffuse_alt.png").c_str());
  saveImageVector(phong_img, filename, "phong");
  saveImageVector(phong_img_alt, filename, "phong_alt");
  saveImageVector(shadow_img, filename, "shadow");
}

void RayTracer1b::render(const Scene1b& scene, const ImagePlane& image_plane, int max_bounce, bool output_intermediates)
{
  void computeSurfaceParameters(const Triangle* triangle, const float2& barycentric, float3& n, float2& texCoords);
  float3 phong(const float3& p, const float3& n, const float3& v, const float3& k_d, const float3& k_s, float shininess, const float3& lightPos, const float3& lightColor);
  bool inShadow(const float3& p, const float3& lightPos, const Scene1b& scene);
  float3 traceRay(const Scene1b& scene, const Ray& ray, int bounce, int max_bounce);

  int w = output_image.width();
  int h = output_image.height();
  size_t num_lights = scene.getLightList().size();

  if (output_intermediates)
  {
    normal_img = surface<R8G8B8A8>(w, h, scene.background());
    texcoord_img = surface<R8G8B8A8>(w, h, scene.background());
    texture_img = surface<R8G8B8A8>(w, h, scene.background());
    texture_img_alt = surface<R8G8B8A8>(w, h, scene.background());

    for (int i = 0; i < num_lights; i++)
    {
      phong_img.emplace_back(w, h, scene.background());
      phong_img_alt.emplace_back(w, h, scene.background());
      shadow_img.emplace_back(w, h, scene.background());
    }
  }

  float progress = 0.0f;

  for (int y = 0; y < h; y++)
  {
    float p = std::trunc(100.0f * (y + 1) / h);
    if (p != progress)
    {
      progress = p;
      std::cout << "\rProgress: " << std::setw(2) << std::setfill('0') << std::fixed << std::setprecision(0) << progress << "%";
    }

    for (int x = 0; x < w; x++)
    {
      float3 dir = normalize(image_plane.a + static_cast<float>(x) * image_plane.c + static_cast<float>(y) * image_plane.d);
      float3 o = image_plane.b;
      Ray ray(o, dir);

      float3 color = traceRay(scene, ray, 0, max_bounce);

      if (output_intermediates)
      {
        const Triangle* tri;
        float t;
        float2 b;
        if (scene.intersectWithRay(tri, t, ray, b))
        {
          float3 p = ray.origin + t * ray.direction + 0.0001f * tri->triangleNormal;

          float3 n;
          float2 tc;
          computeSurfaceParameters(tri, b, n, tc);
          texcoord_img(x, y) = float4(tc.x, tc.y, 0.0f, 1.0f);
          normal_img(x, y) = float4(0.5f * (float3(1.0f) + n), 1.0f);

          PhongMaterial m = scene.materialParameters(tri->materialIndex, tc);
          texture_img(x, y) = float4(clamp(m.k_d), 1);
          PhongMaterial m2 = scene.materialParameters(tri->materialIndex, b);
          texture_img_alt(x, y) = float4(clamp(m2.k_d), 1);

          for (int i = 0; i < num_lights; ++i)
          {
            const Pointlight& light = scene.getLightList()[i];
            float3 v = -normalize(ray.direction);
            phong_img[i](x, y) = phong(p, n, v, m.k_d, m.k_s, m.shininess, light.pos, light.color);
            phong_img_alt[i](x, y) = phong(p, n, v, m2.k_d, m2.k_s, m2.shininess, light.pos, light.color);

            float s = inShadow(p, light.pos, scene) ? 0.0f : 1.0f;
            shadow_img[i](x, y) = float4(s, s, s, 1.0f);
          }
        }
      }

      output_image(x, y) = clamp(color);
    }
  }
  std::cout << std::endl;
}
