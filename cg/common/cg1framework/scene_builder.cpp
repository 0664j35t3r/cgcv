#include <vector>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include "config_utils.h"
#include "scene_builder.h"

using namespace CgcvConfig;

namespace
{
  template <typename T>
  T ref(const std::vector<T>& c, int i)
  {
    return i > 0 ? c[i - 1] : c[c.size() + i];
  }

  template <typename T>
  T ref(const std::vector<T>& c, int i, const T& default_value)
  {
    return i > 0 ? c[i - 1] : (i == 0 ? default_value : c[c.size() + i]);
  }
}

void SceneBuilder::Load(SceneBuilder& builder, CgcvConfig::Container& root)
{
  CgcvConfig::Container& scene = *root.getContainer("scene");

  float3 background = getVector(scene, "background", float3(0.6f, 0.8f, 1.0f));
  builder.SetBackground(background);

  CgcvConfig::Container* cam = root.getContainer("camera");
  float3 eye = getVector(*cam, "eye", float3(0.0f, 0.0f, 1.0f));
  float3 lookat = getVector(*cam, "lookat", float3(0.0f, 0.0f, 0.0f));
  float3 up = getVector(*cam, "up", float3(0.0f, 1.0f, 0.0f));
  float f = getAttribute<float>(*cam, "f", 0.1f);
  float beta = getAttribute<float>(*cam, "beta", 60.0f) * math::constants<float>::pi() / 180.0f;

  builder.SetCamera(eye, lookat, up, f, beta);

  std::unordered_map<std::string, int> material_map;
  size_t num_materials = scene.getNumElements("material");
  for (size_t i = 0; i < num_materials; i++)
  {
    const Container& mat = *scene.getContainer("material", i);

    std::string name = mat.getAttribute<std::string>("name");
    bool reflecting = getAttribute<bool>(mat, "reflection", false);

    float3 k_d = getVector(mat, "k_d", reflecting ? float3(0.0f, 0.0f, 0.0f) : float3(0.8f, 0.8f, 0.8f));
    float shininess = getAttribute<float>(mat, "shininess", 1.0f);
    float3 k_s = getVector(mat, "k_s", reflecting ? float3(1.0f, 1.0f, 1.0f) : float3(0.0f, 0.0f, 0.0f));
    std::string tex = getAttribute<std::string>(mat, "texture", "");

    if (!tex.empty())
      material_map.insert(std::make_pair(name, builder.AddTexturedMaterial(tex.c_str(), k_s, shininess)));
    else if (reflecting)
      material_map.insert(std::make_pair(name, builder.AddReflectiveMaterial(k_d, k_s)));
    else
      material_map.insert(std::make_pair(name, builder.AddUntexturedMaterial(k_d, k_s, shininess)));
  }

  size_t num_objects = scene.getNumElements("object");

  for (size_t i = 0; i < num_objects; i++)
  {
    auto obj = scene.getContainer("object", i);
    auto src = obj->getAttribute<std::string>("src");
    std::cout << "loading " << src << std::endl;

    float3x4 M = getMatrix(*obj, "transform", float3x4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f));
    float3x3 M_n = inverse(float3x3(M._11, M._21, M._31, M._12, M._22, M._32, M._13, M._23, M._33));

    std::vector<float3> v;
    std::vector<float3> vn;
    std::vector<float2> vt;
    int material_index = -1;

    std::ifstream file(src.c_str());
    std::string cmd;
    while (file >> cmd)
    {
      if (cmd[0] == '#')
      {
        while (file && file.get() != '\n')
          ;
      }
      else if (cmd == "v")
      {
        float3 p;
        file >> p.x >> p.y >> p.z;
        v.push_back(M * float4(p, 1.0f));
      }
      else if (cmd == "vn")
      {
        float3 n;
        file >> n.x >> n.y >> n.z;
        vn.push_back(normalize(M_n * n));
      }
      else if (cmd == "vt")
      {
        float2 t;
        file >> t.x >> t.y;
        t.y = 1.0f - t.y;
        vt.push_back(t);
      }
      else if (cmd == "f")
      {
        int indices[3][3];
        for (int i = 0; i < 3; ++i)
        {
          file >> indices[i][0];
          if (file.get() == '/')
          {
            if (file.peek() != '/')
            {
              file >> indices[i][1];
              if (file.get() == '/')
                file >> indices[i][2];
              else
                indices[i][2] = 0;
            }
            else
            {
              file.get();
              indices[i][1] = 0;
              file >> indices[i][2];
            }
          }
          else
            indices[i][1] = indices[i][2] = 0;
        }

        auto v1 = ref(v, indices[0][0]);
        auto v2 = ref(v, indices[1][0]);
        auto v3 = ref(v, indices[2][0]);
        auto n = normalize(cross(v2 - v1, v3 - v1));
        auto vn1 = ref(vn, indices[0][2], n);
        auto vn2 = ref(vn, indices[1][2], n);
        auto vn3 = ref(vn, indices[2][2], n);
        auto vt1 = ref(vt, indices[0][1], float2(0.0f, 0.0f));
        auto vt2 = ref(vt, indices[1][1], float2(0.0f, 0.0f));
        auto vt3 = ref(vt, indices[2][1], float2(0.0f, 0.0f));

        builder.AddTriangle(v1, v2, v3, vn1, vn2, vn3, vt1, vt2, vt3, material_index);
      }
      else if (cmd == "usemtl")
      {
        std::string name;
        file >> name;
        auto f = material_map.find(name);
        if (f != end(material_map))
          material_index = f->second;
        else
          material_index = -1;
      }
    }
  }

  size_t num_lights = scene.getNumElements("pointlight");

  for (size_t j = 0; j < num_lights; ++j)
  {
    const Container& light = *scene.getContainer("pointlight", j);

    auto pos = getVector(light, "position", float3(0.0f));
    float3 color = getVector(light, "color", float3(1.0f));

    builder.AddPointlight(pos, color);
  }
}
