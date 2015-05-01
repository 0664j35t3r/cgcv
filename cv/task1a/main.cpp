#include <iostream>
#include "cfg_config.h"
#include <fstream>

#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"

#define FULL_VERSION 1

using namespace std;
using namespace CgcvConfig;
using namespace cv;

float round(float d)
{
  return std::floor(d + 0.5);
}

int main(int argc, char *argv[])
{
  printf("CV/task1a framework version 1.0\n");  // DO NOT REMOVE THIS LINE!!!

  if (argc != 2) {
    printf("Usage: ./cvtask1a <config-file>\n");
    return -1;
  }
  try
  {
    // load config
    Config* cfg = Config::open(argv[1]);
      
    // get number of testcases
    size_t num_testcases = cfg->getNumElements("testcase");
    for(size_t i = 0; i < num_testcases; i++)
    {
      Testcase *testcase = cfg->getTestcase(i);
      std::cout << std::endl << "running testcase " << testcase->getAttribute<std::string>("name") << std::endl;
     
      // get parameters
      std::string input_filename = testcase->getAttribute<std::string>("input");
      std::string lut_filename = testcase->getAttribute<std::string>("lut");
      std::string outputrgb_filename = testcase->getAttribute<std::string>("outputrgb");
      std::string outputcomic_filename = testcase->getAttribute<std::string>("outputcomic");
      std::string outputedge_filename = testcase->getAttribute<std::string>("outputedge");

      int diameter = testcase->getAttribute<int>("diameter");
      double sigma_color = testcase->getAttribute<double>("sigmacolor");
      double sigma_space = testcase->getAttribute<double>("sigmaspace");
      double threshold1 = testcase->getAttribute<double>("threshold1");
      double threshold2 = testcase->getAttribute<double>("threshold2");
      int aperture_size = testcase->getAttribute<int>("aperture_size");
      int morph_iter = testcase->getAttribute<int>("morph_iter");

      // read image   
      Mat image = imread(input_filename.c_str());
      if(!image.data)
      {
        cout << "Error reading file: " << input_filename.c_str() << endl;
        assert(0);
      }

      // read LUT
      FILE* lutfile;
      lutfile = fopen(lut_filename.c_str(), "r");

      if(lutfile == NULL)
      {
        cout << "Error reading file: " << lut_filename.c_str() << endl;
        fclose(lutfile);
        assert(0);
      }
    
      float b, g, r;
      vector<Vec3f> lut;
      while(fscanf(lutfile, "%f, %f, %f\n", &b, &g, &r) != EOF)
        lut.push_back(Vec3f(b,g,r));

      fclose(lutfile);

      // setup variables
      Mat image_out(image.size(), image.type(), Scalar::all(0));
      Mat image_gray(image.size(), CV_8UC1, Scalar::all(0));
      Mat image_colors(image.size(), CV_8UC3, Scalar::all(0));
      Mat image_filtered(image.size(), CV_8UC3, Scalar::all(0));
      Mat image_edge(image.size(), CV_8UC1, Scalar::all(0));



      // TODO: COMICIFICATION
      // NOTE: Use the given variables!
      //------------------------------------------------------------------------
      // 1) Perform bilateral filtering on the input image. The variables are
      //    defined in diameter, sigma_color and sigma_space.
      cv::bilateralFilter(image, image_filtered, diameter, sigma_color, sigma_space);
      
      //------------------------------------------------------------------------
      // 2) Convert the original image to gray using the convert type CV_BGR2GRAY
      cv::cvtColor(image, image_gray, CV_BGR2GRAY);
      
      //------------------------------------------------------------------------
      // 3) Apply canny edge detector on gray image. Threshold 1, 2 and aperture
      //    size are defined.
      cv::Canny(image_gray,image_edge, threshold1, threshold2, aperture_size);
      
      //------------------------------------------------------------------------
      // 4) Perform morphological operation dilation by morph_iter times.
      cv::dilate(image_edge, image_edge, Mat(), cv::Point(-1,-1), morph_iter);
      
      //------------------------------------------------------------------------
      // 5) Perform comicification on filtered image. Use lut and get smallest 
      //    euklidian distance from pixels to lut. Take entry from lut as new 
      //    pixel color.
      
      // Iterate over all pixels in the image (2D array)
      for(int x = 0; x != image_filtered.cols; x++)
      {
        for(int y = 0; y != image_filtered.rows; y++)
        {
          Vec3b colors = lut.at(0);
          char rgb[3];
          rgb[0] = (char)colors.val[0];
          rgb[1] = (char)colors.val[1];
          rgb[2] = (char)colors.val[2];
          
          // take the first element for starting in order to compare 
          // all approximations and get the minium
          Vec3b filtered_pixel = image_filtered.at<cv::Vec3b>(cv::Point(x,y));
          double minimum = std::sqrt(
            std::pow((double)filtered_pixel[0] - (double)rgb[0],(double)2) + 
            std::pow((double)filtered_pixel[1] - (double)rgb[1],(double)2) + 
            std::pow((double)filtered_pixel[2] - (double)rgb[2],(double)2));
          
          // iterate over the image
          for(auto it = lut.begin(); it != lut.end(); it++)
          {
            double lut_2 = sqrt(
              pow((double)(*it)[0] - (double)filtered_pixel[0],2.) + 
              pow((double)(*it)[1] - (double)filtered_pixel[1],2.) + 
              pow((double)(*it)[2] - (double)filtered_pixel[2],2.));
            
            // if minimum the minimum is found after all iterations.
            // the colors will be put in it
            if(minimum > lut_2)
            {
              minimum = lut_2;
              // safe color information temporarly
              rgb[0] = (char)(*it)[0];
              rgb[1] = (char)(*it)[1];
              rgb[2] = (char)(*it)[2];
            }
          }
          
          // save them into image_colors
          image_colors.at<Vec3b>(cv::Point(x,y))[0] = rgb[0];
          image_colors.at<Vec3b>(cv::Point(x,y))[1] = rgb[1];
          image_colors.at<Vec3b>(cv::Point(x,y))[2] = rgb[2];
        }
      }
      
    //      std::cout << "cols" << image_filtered.cols << std::endl;
    //      std::cout << "rows" << image_filtered.rows << std::endl;
      
      //------------------------------------------------------------------------
      // 6) Final image: combine the image with detected edges and comicificated
      //    image. 
      //    Note: if needed, use round to transform float to uchar.
      for(int x = 0; x != image_filtered.cols; x++)
      {
        for(int y = 0; y != image_filtered.rows; y++)
        { 
          // if image_edge at one pixel is 0, do...
          if((image_edge.at<uchar>(cv::Point(x,y)))  == 0)
          {
            image_out.at<Vec3b>(cv::Point(x,y))[0] = image_colors.at<Vec3b>(cv::Point(x,y))[0];
            image_out.at<Vec3b>(cv::Point(x,y))[1] = image_colors.at<Vec3b>(cv::Point(x,y))[1];
            image_out.at<Vec3b>(cv::Point(x,y))[2] = image_colors.at<Vec3b>(cv::Point(x,y))[2];
          }
          else
          {
            // ... else set them to zero
            image_out.at<Vec3b>(cv::Point(x,y))[0] = 0;
            image_out.at<Vec3b>(cv::Point(x,y))[1] = 0;
            image_out.at<Vec3b>(cv::Point(x,y))[2] = 0;
          }
        }
      }
      // write output images
      imwrite(outputrgb_filename.c_str(),image_out);
      imwrite(outputcomic_filename.c_str(),image_colors);
      imwrite(outputedge_filename.c_str(),image_edge);

      // cleanup
      delete(testcase);

      std::cout << "finished!" << std::endl;
    }
    delete cfg;
    std::cout << std::endl;
  }
  catch(std::exception &ex)
  {
    std::cout << "an exception occured:" << std::endl;
    std::cout << ex.what() << std::endl;
    return -2;
  }
  return 0;
}
