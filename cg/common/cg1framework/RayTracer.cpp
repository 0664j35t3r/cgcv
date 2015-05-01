#include "RayTracer.h"

RayTracer::RayTracer(int width, int height)
    : output_image(width, height, R8G8B8A8(0, 0, 0, 255))
{
}

void RayTracer::SaveOutput(const std::string& filename)
{
  output_image.save((filename + ".png").c_str());
}
