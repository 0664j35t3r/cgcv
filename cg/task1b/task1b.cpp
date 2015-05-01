#include "RayTracer1b.h"
#include "Scene1b.h"

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


  // For now, just return the top left texel.
  return texture(0, 0).toFloat4();
}

float3 phong(const float3& p, const float3& n, const float3& v, const float3& k_d, const float3& k_s, float shininess, const float3& lightPos, const float3& lightColor)
{
//TODO:
// Implement the phong illumination model as described in the assignment
// document to determine the illumination of the given surface point
// with the given material parameters by the given lightsource.
//


  // For now, just return the diffuse material color.
  return k_d;
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


  // For now, just say there is no shadow.
  return false;
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


  // For now, just return white.
  return float3(1.0f, 1.0f, 1.0f);
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


  // For now, just return the background color.
  return scene.background();
}
