#include <complex>
#include <opencv2/core/types_c.h>

#include "RayTracer1b.h"
#include "Scene1b.h"

using std::cout;
using std::endl;
using std::cos;

float3 traceRay(const Scene1b& scene, const Ray& ray, int bounce, int max_bounce);

void computeSurfaceParameters(const Triangle* triangle, const float2& barycentric, float3& n, float2& texCoords)
{
//TODO:
// Compute the interpolated surface normal vector and texture coordinates for
// a point on a triangle given by its barycentric coordinates. barycentric.x
// and barycentric.y are the barycentric coordinates of the point with respect
// to the second and third vertex.
//
// access the vertex normals with
//    triangle->normal1, triangle->normal2, and triangle->normal3
// access the vertex texture coordinates with
//    triangle->texture1, triangle->texture2, and triangle->texture3
//

  // For now, we just set the surface normal to the normal of the triangle and
  // the texture coordinates to the barycentric coordinates of the hitpoint.
  n  = (1 - barycentric.x - barycentric.y) * triangle->normal1 + 
    barycentric.x * triangle->normal2 + 
    barycentric.y * triangle->normal3;
  
  texCoords = (1 - barycentric.x - barycentric.y) * triangle->texture1 + 
    barycentric.x * triangle->texture2 + 
    barycentric.y * triangle->texture3;
}

float4 sampleTexture(const float2& texCoords, const surface<R8G8B8A8>& texture)
{
//TODO:
// Implement texture sampling using bilinear interpolation. The texture
// coordinates of each texel shall correspond to the centers of the cells
// of a grid evenly subdividing the [0, 1] × [0, 1] texture coordinate space
// into texture.width() × texture.height() cells while the coordinates
// (0, 0) and (1, 1) shall correspond to the top-left and bottom-right
// corners of the texture respectively. Texel accesses that fall
// outside the image shall be clamped.
//
// access the texture image with
//    texture(x, y).toFloat4()
//
  float x = texCoords.x;
  float y = texCoords.y;
  
  int px = (int)(x * texture.width());
  int py = (int)(y * texture.height());
    
  R8G8B8A8 c_11 = texture(px    , py    );
  R8G8B8A8 c_12 = texture(px + 1, py    );
  R8G8B8A8 c_21 = texture(px    , py + 1);
  R8G8B8A8 c_22 = texture(px + 1, py + 1);
  
  float intersec = x * (float)texture.width() - (float)px;
  
  R8G8B8A8 obn = R8G8B8A8( intersec * c_11.r() + (1 - intersec) * c_12.r(),
                           intersec * c_11.g() + (1 - intersec) * c_12.g(),
                           intersec * c_11.b() + (1 - intersec) * c_12.b(),
                           intersec * c_11.a() + (1 - intersec) * c_12.a() ); 
  

  R8G8B8A8 untn = R8G8B8A8( intersec * c_21.r() + (1 - intersec) * c_22.r(),
                            intersec * c_21.g() + (1 - intersec) * c_22.g(),
                            intersec * c_21.b() + (1 - intersec) * c_22.b(),
                            intersec * c_21.a() + (1 - intersec) * c_22.a() );

  
  intersec = y * (float)texture.height() - (float)py;
  
  R8G8B8A8 interpolated = R8G8B8A8( intersec * obn.r() + (1 - intersec) * untn.r(),
                                    intersec * obn.g() + (1 - intersec) * untn.g(),
                                    intersec * obn.b() + (1 - intersec) * untn.b(),
                                    intersec * obn.a() + (1 - intersec) * untn.a() ); 
  
  return interpolated.toFloat4();
}

float3 phong(const float3& p, const float3& n, const float3& v, const float3& k_d, const float3& k_s, float shininess, const float3& lightPos, const float3& lightColor)
{
//TODO:
// Implement the phong illumination model as described in the assignment
// document to determine the illumination of the given surface point
// with the given material parameters by the given lightsource.
//
  // calculate the angle theta
  // calculate c_d
  // https://www.ltcconline.net/greenl/courses/107/vectors/dotcros.htm  
  float3 vector_l = normalize(static_cast<float3>(lightPos - p));
  float3 norm = normalize(n);
  float theta = dot(n, vector_l);
  
  float3 c_d = k_d * MAX(theta, 0);
  
  // calculate c_s
  //                                  r = 2n(n l)  - l , 3. foliensatz s54
  float3 r = float3(2 * norm * cross(norm, vector_l) - vector_l);
  //  float alpha = dot(normalize(v_norm), normalize(vector_r));
  float alpha = dot(normalize(v), normalize(r));
  
  float3 c_s(0,0,0);
  if(theta)
    c_s = k_s * pow(MAX(alpha, 0), shininess);
    
  return (c_s + c_d) * lightColor;
}

bool inShadow(const float3& p, const float3& lightPos, const Scene1b& scene)
{
//TODO:
// Determine if the given point p lies in the shadow of the given light by
// casting a ray towards the lightsource.
//
// Use the method
//    bool intersectWithRay(const Triangle*& tri, float& t, const Ray& ray);
// of the scene object to compute whether a ray intersects the scene
// geometry.
//
  const Triangle* intersectedTriangle;
  Ray ray(p, lightPos - p);
  float t;
  return scene.intersectWithRay(intersectedTriangle, t, ray);
//  return false;
}

float3 shade(const Scene1b& scene, const float3& p, const float3& n, const float3& v, 
  const float3& k_d, const float3& k_s, float shininess, int bounce, int max_bounce)
{
//TODO:
// Compute the shading of the given surface point with the given material
// parameters. A shininess value of infinity indicates that the surface acts
// as a mirror.
//
// access the list of lights in the scene with
//    scene.getLightList()
// check whether shininess is infinity using
//    std::isinf(shininess)
//
  // float3 phong(const float3& p, const float3& n, const float3& v, const float3& k_d, const float3& k_s, float shininess, const float3& lightPos, const float3& lightColor)
  float3 result(0,0,0);

  if(std::isinf(shininess))
  {
    return float3(1.0f ,1.0f ,1.0f);
  }
  
  for(auto& light : scene.getLightList())
  {
    if(!inShadow(p, light.pos, scene))
      result += phong(p, n, v, k_d, k_s, shininess, light.pos, light.color);
  }
  
  return result;
}

float3 traceRay(const Scene1b& scene, const Ray& ray, int bounce, int max_bounce)
{
// TODO:
// Compute the color of the given ray. If the ray hits an object in the scene,
// return the color of the shaded object at the hit location, otherwise, return
// the scene's background color.
//
// Use the method
//    bool intersectWithRay(const Triangle*& tri, float& t, const Ray& ray, float2& barycentric)
// of the scene object to compute the point of intersection between a ray and
// the scene geometry.
//
// If an intersection is found, offset the computed point of intersection by
// the distance epsilon along the normal of the hit triangle to avoid problems
// resulting from the computed point not exactly lying in the plane of the
// triangle due to floating point rounding errors
//
// access the triangle normal with
//    tri->triangleNormal
//
// Use the method
//    PhongMaterial materialParameters(int materialIndex, const float2& texCoords)
// of the scene object to get the material parameters at a given surface point.
//
// find the index of the material associated with a given triangle using
//    tri->materialIndex
//
  static const float epsilon = 0.0001f;
  const Triangle* triangle;
  float t;
  float2 barycentric;
  float3 n;
  float2 texCoords;
  
  bool intersect = scene.intersectWithRay(triangle, t, ray, barycentric);
  if (!intersect)
    return scene.background();
  
  computeSurfaceParameters(triangle, barycentric, n, texCoords);
  PhongMaterial material = scene.materialParameters(triangle->materialIndex, texCoords);
  
  return shade(scene, ray.origin + ray.direction * t + triangle->triangleNormal * epsilon, 
    n, -ray.direction, material.k_d, material.k_s, material.shininess, bounce, max_bounce); 
}
