#include <complex>

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
  n = triangle->triangleNormal;
  texCoords = barycentric;
  
  math::float3 n1 = triangle->normal1;
  math::float3 n2 = triangle->normal2;
  math::float3 n3 = triangle->normal3;
  
  math::float2 t1 = triangle->texture1;
  math::float2 t2 = triangle->texture2;
  math::float2 t3 = triangle->texture3;
  
  n  = (1 - barycentric.x - barycentric.y) * n1 + barycentric.x * n2 + barycentric.y * n3;
  n.normalize();
  
  texCoords = (1 - barycentric.x - barycentric.y) * t1 + barycentric.x * t2 + barycentric.y * t3;
  
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
  
  
  int xi = (int)(x * (float)texture.width());
  int yi = (int)(y * (float)texture.height());
  
  int x11 = xi;
  int y11 = yi;
  
  int x12 = xi + 1;
  int y12 = yi;
  
  int x21 = xi;
  int y21 = yi + 1;
  
  int x22 = xi + 1;
  int y22 = yi + 1;
  
  R8G8B8A8 c11 = texture(x11, y11);
  R8G8B8A8 c12 = texture(x12, y12);
  R8G8B8A8 c21 = texture(x21, y21);
  R8G8B8A8 c22 = texture(x22, y22);
  
  
  float intersec = x * (float)texture.width() - (float)x11;
  
  R8G8B8A8 obn = R8G8B8A8(  intersec * c11.r() + (1 - intersec) * c12.r(),
                            intersec * c11.g() + (1 - intersec) * c12.g(),
                            intersec * c11.b() + (1 - intersec) * c12.b(),
                            intersec * c11.a() + (1 - intersec) * c12.a() ); 
  

  R8G8B8A8 untn = R8G8B8A8( intersec * c21.r() + (1 - intersec) * c22.r(),
                            intersec * c21.g() + (1 - intersec) * c22.g(),
                            intersec * c21.b() + (1 - intersec) * c22.b(),
                            intersec * c21.a() + (1 - intersec) * c22.a() );

  
  intersec = y * (float)texture.height() - (float)y11;
  
  R8G8B8A8 interpolated = R8G8B8A8( intersec * obn.r() + (1 - intersec) * untn.r(),
                                    intersec * obn.g() + (1 - intersec) * untn.g(),
                                    intersec * obn.b() + (1 - intersec) * untn.b(),
                                    intersec * obn.a() + (1 - intersec) * untn.a() ); 
  
  // For now, just return the top left texel.
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
  float3 vector_l = float3(lightPos - p).normalize();
  // https://www.ltcconline.net/greenl/courses/107/vectors/dotcros.htm  
  float theta = dot(n, vector_l);
    
  float3 c_d = k_d * theta;//cos(theta);
  
  // calculate c_s
  //                                              r = 2n(n l)  - l , 3. foliensatz s54
  float3 vector_r = float3( 2 * n * cross(n, vector_l) - vector_l).normalize();
  float3 v_norm = v;
  float alpha = dot(v_norm.normalize(), vector_r.normalize());
  
  float3 c_s(0,0,0);
  if(theta)
    c_s = k_s * pow(alpha, shininess);
  
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

  // example intersection test
  const Triangle* intersectedTriangle;
  Ray ray(lightPos, p - lightPos);
  float t;
  // For now, just say there is no shadow.
  return scene.intersectWithRay(intersectedTriangle, t, ray);
}

float3 shade(const Scene1b& scene, const float3& p, const float3& n, const float3& v, const float3& k_d, const float3& k_s, float shininess, int bounce, int max_bounce)
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
  std::vector<Pointlight> vec_color = scene.getLightList();
  for(auto &light : vec_color)
    result += phong(p, n, v, k_d, k_s, std::isinf(shininess), light.pos, light.color);
  
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
  
  const Triangle* tri;
  float2 texcords;
  float t;
  
//  tri->triangleNormal + epsilon;
  if(!scene.intersectWithRay(tri, t, ray))
    return scene.background();
  
  PhongMaterial material = scene.materialParameters(tri->materialIndex, texcords);
//  material.k_d
//  material.k_s
//  material.shininess
//  


  // For now, just return the background color.
}
