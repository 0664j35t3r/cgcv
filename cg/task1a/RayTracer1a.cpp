#include <iostream>
#include <iomanip>
#include <sys/stat.h>
#include <sys/types.h>
#include <sstream>
#include "config_utils.h"
#include "RayTracer1a.h"
#include "Scene1a.h"

#ifdef _WIN32
#include <direct.h>
#endif

const char* taskname = "cgtask1a";
const char* version_string = "CG/task1a framework version 1.2";

void run_testcase(CgcvConfig::Testcase* testcase, const std::string& output, int width, int height)
{
  std::cout << "setting up scene..." << std::endl;

  Scene1a scene(*testcase);
  RayTracer1a rayTracer(width, height);

  std::cout << "rendering..." << std::endl;

  rayTracer.render(scene, output);

  std::cout << "saving output to '" << output << "'..." << std::endl;
  std::stringstream dir;
  dir << "output/" << output;
#ifdef _WIN32
  _mkdir(dir.str().c_str());
#else
  mkdir(dir.str().c_str(), 0777);
#endif
  rayTracer.SaveOutput(dir.str() + "/" + output);

  std::cout << "done!" << std::endl;
}

RayTracer1a::RayTracer1a(int width, int height) : RayTracer(width, height)
{
}

void RayTracer1a::render(const Scene1a& scene, const std::string& output)
{
  surface<float> tmpSurface(output_image.width(), output_image.height(), 0.0f);

  void render(const Scene1a&, surface<float>&);

  render(scene, tmpSurface);

  for (int pixel_y = 0; pixel_y < tmpSurface.height(); pixel_y++)
  {
    for (int pixel_x = 0; pixel_x < tmpSurface.width(); pixel_x++)
    {
      if (tmpSurface(pixel_x, pixel_y) != 0.0f)
      {
        output_image(pixel_x, pixel_y) = scene.scaleDistance(tmpSurface(pixel_x, pixel_y));
      }
    }
  }
}
