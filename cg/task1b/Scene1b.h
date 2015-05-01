

#ifndef SCENE_1B_H_
#define SCENE_1B_H_

#pragma once

#include "Scene.h"
#include "surface.h"
#include <limits>
#include <string>
#include <memory>
#include <unordered_map>

struct ImagePlane
{
  float3 a, b, c, d;
};

struct PhongMaterial
{
  float3 k_d;
  float3 k_s;
  float shininess;
  PhongMaterial() {}
  PhongMaterial(const float3& diffuse, const float3& specular, float shininess) : k_d(diffuse), k_s(specular), shininess(shininess) {}
};

class Material
{
protected:
  Material() = default;
  Material(const Material&) = default;
  Material& operator=(const Material&) = default;

public:
  virtual PhongMaterial getLocalPhongParameters(const float2& texCoords) const = 0;
  virtual ~Material(){};
};

class Scene1b : public Scene
{
private:
  std::unique_ptr<Material> defaultMaterial;
  std::unordered_map<int, std::unique_ptr<Material>> materials;

  int AddReflectiveMaterial(const float3& diffuse, const float3& specular);
  int AddUntexturedMaterial(const float3& diffuse, const float3& specular, float exp);
  int AddTexturedMaterial(const char* tex, const float3& specular, float exp);

public:
  Scene1b(CgcvConfig::Container& root);

  Scene1b(const Scene1b&) = delete;
  Scene1b& operator=(const Scene1b&) = delete;

  PhongMaterial materialParameters(int materialIndex, const float2& texCoords) const;
};

#endif /* SCENE1B_H_ */
