#include "Scene1a.h"
#include "RayTracer1a.h"

/**
TODO implement this function
*/
void render(const Scene1a& scene, surface<float>& output_image)
{
  /*
  TODO:
  setup view parameters as described in the document
  loop over every pixel, find the intersection, and store the t into the output image

  usefull functions/variables:

  scene->getCamera()        -- Camera of the scene, use this object to calculate the view parameters
  camera->eye               -- position of the camera
  camera->lookat            -- lookat position of camera
  camera->up                -- up vector of camera
  camera->f                 -- focal distance
  camera->beta              -- vertical field of view

  normalize(float3)         -- function for normalizing vectors
  cross(float3,float3)      -- calculates the cross product of 2 vectors
  Ray(float3,float3)        -- use this to construct a new ray -- parameters Ray(origin point of ray, ray direction) ray direction should be normalized!

  scene.intersectWithRay(intersectedTriangle, t, ray);
  -- use this function for intersection tests
  -- out const Triangle* intersectedTriangle  -- this variable will contain the intersected triangle
  -- out float t                              -- this variable will contain the ray parameter for the point of intersection (use this to color your output_image)
  -- in Ray ray                               -- this variable should contain the ray for intersection testing
  -- return Intersection                      -- Intersection.intersected should be true if an intersection happened


  */
  
   // get camera
  const Camera* camera = scene.getCamera();
  
  // view parameter setup
  double3 eye = camera->eye;
  double3 lookat = camera->lookat; 
  double3 up = camera->up;    // vector, which normal to plookat
  
  double beta =(double)camera->beta; // winkel zwichen f und sehstrahlen 
  double f = (double)camera->f;
  
  double3 w = normalize(eye - lookat);
  double3 u = normalize(cross(up, w));
  double3 v = cross(w, u);
  
  // loop over image and test for intersections
  int width = output_image.width();
  int height = output_image.height();
  double h_s = 2.0f * f * std::tan(beta/2.0f);
  std::cout<<"width:"<<width<<std::endl;
  std::cout<<"height:"<<height<<std::endl;
  double alpha = (double)width/height;
  double w_s = alpha * h_s;
  
  
  // example intersection test
  const Triangle* intersectedTriangle;
  Ray ray(eye, double3(.0, .0, 1.0));
  float t;  
  
  // center of image plane
  double3 center = eye + f * (-w);
  
  // reference point
  double3 left_top_corner = center + (h_s / 2) * v + (w_s / 2) * (-u);
  double3 p1 = double3(.0, .0, .0);
  
  for(int pixel_x = 0; pixel_x < width; pixel_x++)
  {
    for(int pixel_y = 0; pixel_y < height; pixel_y++)
    {
      p1 = double3(left_top_corner) + double3((((double)pixel_x + 0.5)/width) * w_s * (u)) + 
                double3((((double)pixel_y + 0.5)/height) * h_s * (-v));
      
      // normal vector for the following function
      ray.direction = normalize(p1 - eye);
      
      // bool Scene::intersectWithRay(const Triangle*& intersection, float& t, Ray ray) const
      if(scene.intersectWithRay(intersectedTriangle, t, ray))
      {
        output_image(pixel_x, pixel_y) = t;
      }
    }
  }
    
  
//  // get camera
//  const Camera* camera = scene.getCamera();
//
//  // view parameter setup
//
//  // loop over image and test for intersections
//  int width = output_image.width();
//  int height = output_image.height();
//
//  // example intersection test
//  const Triangle* intersectedTriangle;
//  Ray ray(float3(0, 0, 0), float3(0, 0, 1));
//  float t;
//  bool intersected = scene.intersectWithRay(intersectedTriangle, t, ray);
//  // if intersection happened store t into output image
//  if (intersected)
//    output_image(0, 0) = t;

}
