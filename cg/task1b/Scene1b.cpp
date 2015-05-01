#include <iostream>
#include <iomanip>
#include <fstream>
#include "Scene1b.h"

float4 sampleTexture(const float2& texCoords, const surface<R8G8B8A8>& texture);

namespace
{
  class UntexturedMaterial : public Material
  {
    PhongMaterial material;

  public:
    UntexturedMaterial(const float3& diffuse, const float3& specular, float shininess) : material(diffuse, specular, shininess) {}
    PhongMaterial getLocalPhongParameters(const float2& texCoords) const { return material; }
  };

  class TexturedMaterial : public Material
  {
    float3 k_s;
    float shininess;
    surface<R8G8B8A8> tex;

  public:
    TexturedMaterial(const char* tex, const float3& k_s, float shininess) : k_s(k_s), shininess(shininess), tex(tex) {}
    PhongMaterial getLocalPhongParameters(const float2& texCoords) const
    {
      float4 diffuseColor = sampleTexture(texCoords, tex);
      return PhongMaterial(float3(diffuseColor.x, diffuseColor.y, diffuseColor.z), k_s, shininess);
    }
  };

  class ReflectiveMaterial : public Material
  {
    float3 k_d, k_s;

  public:
    ReflectiveMaterial(const float3& k_d, const float3& k_s) : k_d(k_d), k_s(k_s) {}
    PhongMaterial getLocalPhongParameters(const float2& texCoords) const
    {
      return PhongMaterial(k_d, k_s, std::numeric_limits<float>::infinity());
    }
  };
}

Scene1b::Scene1b(CgcvConfig::Container& root) : defaultMaterial(new UntexturedMaterial(float3(1.0f), float3(0.0f), 0.0f))
{
  SceneBuilder::Load(*this, root);
  buildBVH();
}

int Scene1b::AddReflectiveMaterial(const float3& diffuse, const float3& specular)
{
  int id = static_cast<int>(materials.size());
  materials.insert(std::make_pair(id, std::unique_ptr<Material>(new ReflectiveMaterial(diffuse, specular))));
  return id;
}
int Scene1b::AddUntexturedMaterial(const float3& diffuse, const float3& specular, float exp)
{
  int id = static_cast<int>(materials.size());
  materials.insert(std::make_pair(id, std::unique_ptr<Material>(new UntexturedMaterial(diffuse, specular, exp))));
  return id;
}
int Scene1b::AddTexturedMaterial(const char* tex, const float3& specular, float exp)
{
  int id = static_cast<int>(materials.size());
  materials.insert(std::make_pair(id, std::unique_ptr<Material>(new TexturedMaterial(tex, specular, exp))));
  return id;
}

PhongMaterial Scene1b::materialParameters(int materialIndex, const float2& texCoords) const
{
  auto found = materials.find(materialIndex);
  if (found == materials.end())
    return defaultMaterial->getLocalPhongParameters(texCoords);
  return found->second->getLocalPhongParameters(texCoords);
}
