#include <iostream>
#include <iomanip>
#include <algorithm>

#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <vector>

#include "cfg_config.h"
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/objdetect/objdetect.hpp"

using namespace std;
using namespace CgcvConfig;
using namespace cv;

#define FULL_VERSION 1
#define PNG ".png"

#define H_DIMENSIONS 18
#define H_BIN_SIZE 10

#define S_DIMENSIONS 3
#define S_BIN_SIZE 86

#define V_DIMENSIONS 3
#define V_BIN_SIZE 86

#define H_WEIGHT 9
#define S_WEIGHT 3
#define V_WEIGHT 1

#define DIMENSIONS (H_DIMENSIONS * S_DIMENSIONS * V_DIMENSIONS)

// H range: 0..179 -> 18 bins - [0, 9], [10, 19], ..., [170, 179]
// S range: 0..255 -> 03 bins - [0,85], [86,171], [172, 255]
// V range: 0..255 -> 03 bins - [0,85], [86,171], [172, 255]


//==============================================================
// drawHist()
//------------------------------------------------------------------------------
// TODO:
//	- Nothing!
//	- Do not change anything here
//==============================================================
void drawHist( string name, MatND input )
{
  cout << "rawHist" << endl;
  int hist_w = 512;
  int hist_h = 400;
  int bin_w = cvRound( ( double )hist_w / input.cols );
  Mat histImage( hist_h, hist_w, CV_8UC3, Scalar( 0, 0, 0 ) );
  Mat hist = input.clone();
  normalize(hist, hist, 0, 255, NORM_MINMAX);

  /// Draw for each channel
  for (int i = 1; i < hist.cols; i++)
  {
    line( histImage, Point( bin_w * ( i - 1 ), hist_h - cvRound( hist.at<float>( i - 1 ) ) ),
      Point( bin_w * ( i ), hist_h - cvRound( hist.at<float>( i ) ) ),
      Scalar( 255, 255, 255 ), 2, 8, 0 );
  }
  imwrite( name, histImage );
}

//==============================================================
// getHistogram(const Mat &lab)
//------------------------------------------------------------------------------
// TODO:
//	- calculate the Histogram for one channel of an Lab image
//      - do not use the function provided by OpenCV
//
//
// Parameters:
//	- lab: one channel of a lab image
//==============================================================
Mat getHistogram( const Mat &lab )
{
  cout << "getHistogram" << endl;
  
  Mat hist = Mat::zeros(1, 256, CV_32FC1);

  for (int iter_y = 0; iter_y < lab.rows; iter_y++)
    for (int iter_x = 0; iter_x < lab.cols; iter_x++)
       hist.at<double>(lab.at<uchar>(iter_x, iter_y)) += 1.0;
  
  return hist;
}

//==============================================================
// matchHistograms()
//------------------------------------------------------------------------------
// TODO:
//      - convert the input images to Lab colorspace (cvtColor)
//	    - split the patch_src and patch_dst in their channels
//      - for the channels a and b:
//              - calculate two histograms, one for the channel from src_image and
//                one for the channel of face_dest
//              - use the function getHistogram(...) (has to be implemented)
//              - normalize the histograms (NORM_L1)
//              - from the normalized histograms calculate the CDF
//              - for each value of the CDF of the patch_src image find the best match
//                in the CDF of the face_dest
//              - map the position of the value of the src_cdf to the positon of the
//                dst_cdf
//              - with the previous generatet LUT replace the values in the
//                src_channel in the whole image with the values of the LUT
//      - merge the channels again
//      - convert the image back to BGR
//      - save the image in matched
//
//==============================================================
void matchHistograms( Mat patch_src, Mat patch_dst, Mat &matched )
{
  cout << "matchHistogram" << endl;
  
  matched = Mat::zeros( matched.size(), CV_8U ); // CV_8U: 0-255
  // convert the input images to Lab colorspace (cvtColor)
  cv::cvtColor(patch_src, patch_src, CV_RGB2Lab);
  cv::cvtColor(patch_dst, patch_dst, CV_RGB2Lab);
  
  // split the patch_src and patch_dst in their channels 
  Mat *channel_src;
  Mat *channel_dst;
  split(patch_src, channel_src);
  split(patch_dst, channel_dst);
  
  Mat a_src = getHistogram(channel_src[1]);
  Mat b_src = getHistogram(channel_src[2]);
  Mat a_dst = getHistogram(channel_dst[1]);
  Mat b_dst = getHistogram(channel_dst[2]);
  
  
  // normalize the histograms (NORM_L1)
  // cv::normalize(src, dst, alpha, beta, NORM_MINMAX, dtype); 
  cv::normalize(a_src, a_src, 0, 255, NORM_MINMAX); 
  cv::normalize(b_src, b_src, 0, 255, NORM_MINMAX); 
  cv::normalize(a_dst, a_dst, 0, 255, NORM_MINMAX); 
  cv::normalize(b_dst, b_dst, 0, 255, NORM_MINMAX); 
  
  // calculate CDF
  cv::equalizeHist(a_src, a_src);
  cv::equalizeHist(b_src, b_src);
  cv::equalizeHist(a_dst, a_dst);
  cv::equalizeHist(b_dst, b_dst);
  
  // map the position of the value of the src_cdf to the positon of the dst_cdf
  std::vector<double> lut_a;
  std::vector<double> lut_b;
  
  // page 6; i = src, j = dst
  double minimum = 0;
  double compare;
  double tmp_i;
  double tmp_j;
  for(int i = 0; i < 256; i++)  
  {
    tmp_i = a_src.at<double>(i);
    for(int j = 0; j < 256; j++)
    {
      tmp_j = a_dst.at<double>(j);
      compare = std::abs(tmp_i - tmp_j);
      if(minimum > compare)
        minimum = compare;
    }
    lut_a.push_back(minimum);
  }
  
  for(int i = 0; i < 256; i++)  
  {
    tmp_i = b_src.at<double>(i);
    for(int j= 0; j < 256; j++)
    {
      tmp_j = b_dst.at<double>(j);
      compare = std::abs(tmp_i - tmp_j);
      if(minimum > compare)
        minimum = compare;
    }
    lut_b.push_back(minimum);
  }
  
  
  // with the previous generate LUT replace the values in the
  // src_channel in the whole image with the values of the LUT
  for (int y = 0; y < patch_src.rows; y++)
  {
    for (int x = 0; x < patch_src.cols; x++)
    {
      channel_src[1].at<uchar>(x, y) = lut_a[channel_src[1].at<uchar>(x, y)];
      channel_src[2].at<uchar>(x, y) = lut_b[channel_src[2].at<uchar>(x, y)];
    }
  }
  
  // merge
  merge(channel_src, 3, patch_src);
  merge(channel_dst, 3, patch_dst);
  
  // convert image back to bgr
  cv::cvtColor(patch_src, patch_src, CV_Lab2BGR);
}

//==============================================================
// getPatches(const Mat &image, vector<Mat> &patches, unsigned int patch_size)
//------------------------------------------------------------------------------
// TODO:
//	- extract the Patches from the original picture
//
//
// Parameters:
//	- image: original picture
//	- patches: vector to fill in the patches
//	- patch_size: size of the patches (normally 32)
//==============================================================

void getPatches( const Mat &image, vector<Mat> &patches, const unsigned int patch_size )
{   
  cout << "get patches" << endl;
  
  int count = 1;
  // create one new patch
  Mat tmp = Mat::zeros( patch_size, patch_size, CV_32FC1 );

  // write the pixels in this patch 
  for (int y = 0; y < image.rows; y = y + patch_size)
  {
    for (int x = 0; x < image.cols; x = x + patch_size)
    {
      tmp = image.colRange(x, x + patch_size).rowRange(y, y + patch_size);
      patches.push_back(tmp);
      
      //  cout << "Y: " << y << " X: " << x << endl;
    }
    cout << count++ << endl;
  } 
  cout << "final" << endl;
}     


//==============================================================
// setPatches( Mat &image, vector<Mat> &patches, unsigned int patch_size)
//------------------------------------------------------------------------------
// TODO:
//	- reconstruct the picture from the patches
//
//
// Parameters:
//	- image: mosaic picture
//	- patches: vector with the patches
//	- patch_size: size of the patches (normally 32)
//================================================================================
void setPatches( Mat &image, vector<Mat> &patches, unsigned int patch_size )
{
  cout << "setPatches" << endl;
  
  for (unsigned y = 1; y < image.rows - 1; y = y + patch_size)
    for (unsigned x = 1; x < image.cols - 1; x = x + patch_size)
      image.at<Vec3f>(x, y) = patches.at(x + y * image.cols / patch_size); // todo durchrechnen!!! 
}

//================================================================================
// retrieveHistogramsFromFile( string file_name )
//--------------------------------------------------------------------------------
// TODO:
//	- Nothing!
//	- Do not change anything here
//================================================================================
Mat retrieveHistogramsFromFile( string file_name )
{
  cout << "retrieveHistogramsFromFile" << endl;
  
  Mat returnValue = Mat::zeros( 0, DIMENSIONS, CV_32FC1 );
  Mat mInput = Mat::zeros( 0, DIMENSIONS, CV_32FC1 );
  mInput = imread( file_name );
  for (int iR = 0; iR < mInput.rows; ++iR)
  {
    Mat tmp = mInput.row( iR );
    tmp.convertTo( tmp, CV_32FC1 );
    Mat converted = Mat::zeros( 1, DIMENSIONS, CV_32FC1 );
    for (unsigned int i = 0; i < DIMENSIONS; ++i)
      converted.at<float>( 0, i ) = tmp.at<float>( 0, 3 * i );
    
    returnValue.push_back( converted );
  }
  return  returnValue;
}

int h_hist_max = -INT_MAX;
int h_hist_min =  INT_MAX;
int s_hist_max = -INT_MAX;
int s_hist_min =  INT_MAX;
int v_hist_max = -INT_MAX;
int v_hist_min =  INT_MAX;
int max_idx    = -INT_MAX;
int min_idx    =  INT_MAX;

float bin_max = 0.0;

//================================================================================
// getHSVHistogram(const Mat &hsv)
//--------------------------------------------------------------------------------
// TODO:
//	- calculate the Histogram for all channels of a HSV image:
//	- look at every pixel and every channel
//	- calculate the area for the channels (H-Channel: 18 areas; S-/V-Channel: 3 areas)
//	- calculate the index for the histogram
//
//	- use the formular provided in the document
//      - do not use the function provided by OpenCV
//
//
// Parameters:
// - hsv: one hsv image
//================================================================================
Mat getHSVHistogram( const Mat &hsv )
{
  cout << "getHSVHistogram" << endl;
  
  Mat hist = Mat::zeros( 1, DIMENSIONS, CV_32FC1 );
  
  // iterate over the orginal image, which will be converted into the HSV space!
  for (int iter_x = 0; iter_x < hsv.cols; iter_x++)
  {
    for (int iter_y = 0; iter_y < hsv.rows; iter_y++)
    {
      // calculate hsv values
      Vec3f pix = hsv.at<Vec3f>(iter_x,iter_y); 
      uchar h = 0;
      if((pix[0] / 10) < 180)
      {
        h = floor(pix[0] / 10);
      }
      uchar s = floor(pix[1] / 86);
      uchar v = floor(pix[2] / 86);
      
      // index(i) = 9h(i)+3s(i)+v(i) increased
      hist.at<float>(0, 9*h + 3*s + v) += 1.0f;
    }
  }
   
  return hist;
}

//================================================================================
// main()
//
//================================================================================
int main( int argc, char *argv[] )
{
  printf( "CV/task1b framework version 1.0\n" ); // DO NOT REMOVE THIS LINE!!!

  if (argc != 2)
  {
    cout << "Usage: ./cvtask1b <config-file>\n" << endl;
    return -1;
  }

  try
  {
    // load config
    const Config* cfg = Config::open( argv[1] );

    // input data
    const string input_query_image = cfg->getAttribute<string>( "in_query_image" );

    // image patches
    const string mosaic_path = cfg->getAttribute<string>( "database_path" );
    const string mosaic_histo = cfg->getAttribute<string>( "database_hist" );
    const unsigned int num_images = cfg->getAttribute<unsigned int>( "database_num_images" );
    const unsigned int mos_size = cfg->getAttribute<unsigned int>( "database_resolution" );

    //KD tree
    const unsigned int KDTreeIPara = cfg->getAttribute<unsigned int>( "KDTreeIndexParams" );
    const unsigned int KDTreeSearchParams = cfg->getAttribute<unsigned int>( "KDTreeSearchParams" );

    // output data
    const string out_01_hsv = cfg->getAttribute<string>( "out_01_hsv" );
    const string out_02_hist = cfg->getAttribute<string>( "out_02_hist" );
    const string out_03_final_rep = cfg->getAttribute<string>( "out_03_final_rep" );
    const string out_04_final = cfg->getAttribute<string>( "out_04_final" );

    cout << "===================================" << endl;
    cout << "               CV Task 1b          " << endl;
    cout << "===================================" << endl;

    // <MAIN PROGRAM>

    //_______________________________________________________
    // read Database Pics
    //-------------------------------------------------------
    // TODO:
    //	- read Database Pics
    //	- save the pictures in mosaic_pieces
    //_______________________________________________________
    vector<Mat> mosaic_pieces;
    cout << "num images: " << num_images << endl;
    
    for(int i = 0; i < num_images; i++)
    {
      const std::string mosaic_peaces_name = mosaic_path + to_string(i) + PNG;
      mosaic_pieces.push_back(imread(mosaic_peaces_name)); // warning 
      
      cout << mosaic_peaces_name << endl;
    }

    Mat mDBHistograms;
    
    vector<Mat> vecPatches;
    Mat mQueryImage = imread( input_query_image );
    Size pic = mQueryImage.size();
    int height = (pic.width / mos_size) * (pic.height / mos_size);

    
    //_______________________________________________________
    // TODO:
    //  - read Database Histo from File to mDBHistograms
    //     use to Function retrieveHistogramsFromFile
    //  - convert query Image into HSV (CV_BGR2HSV)
    //  - save HSV-Image to out_01_hsv
    //  - get Patches from query Image (with mos_size)
    //
    //  Original Picture is stored in mQueryImage 
    //_______________________________________________________
    mQueryImage = Mat::zeros(mQueryImage.size(), CV_8UC3);
    
    Mat mInputHistograms = Mat::zeros( 0, DIMENSIONS, CV_32FC1 );
    mInputHistograms = retrieveHistogramsFromFile(mosaic_histo);
    cvtColor(mQueryImage, mQueryImage, CV_BGR2HSV);
    
    
    imwrite( out_01_hsv, mQueryImage );

    //_______________________________________________________
    // TODO:
    //  - calculate Histo from Patches and
    //    save in mInputHistograms:
    //      Mat for histogram of each patch
    //      / --> histogram
    //      |
    //      v
    //      patches
    //_______________________________________________________
    Mat out_mInputHistograms;
    out_mInputHistograms = Mat::zeros( height, DIMENSIONS, CV_32FC1 );;
    imwrite( out_02_hist, out_mInputHistograms );
    
    getPatches(mQueryImage, vecPatches, mos_size);
    
    //_______________________________________________________
    // TODO:
    //  - set KD-Tree:
    //      use the flann lib
    //      for the distance use FLANN_DIST_EUCLIDEAN
    //_______________________________________________________
    flann::Index flann_index;

    vector<Mat> vecPatches_matched;
    vector<Mat> rep_vecPatches_matched;
    
    flann_index.build(mInputHistograms, cv::flann::SearchParams(162), cvflann::FLANN_DIST_EUCLIDEAN);
    
    
    //_______________________________________________________
    // TODO:
    //	- find nearest neighbor with the kd-tree based
    //	  on the histograms (take care of the right
    //	  color space -> CV_HSV2BGR)
    //	- save the patches in vecPatches_matched
    //	- BONUS: suppress reptetition (look for the 5
    //	  nearest neighbors and take the one, which was
    //	  not used for the longest)
    //	- BONUS: save the patches in rep_vecPatches_matched
    //	- match Histograms (use the function matchHistograms:
    //	  has to be implemented)
    //_______________________________________________________
    Mat out_final_matched = Mat::zeros( mQueryImage.size(), CV_8UC3 );
    Mat rep_out_final_matched = Mat::zeros( mQueryImage.size(), CV_8UC3 );
    for(auto &it : vecPatches)
      flann_index.knnSearch(it, mosaic_pieces, out_final_matched, 5, KDTreeSearchParams);
    
    //_______________________________________________________
    // TODO:
    //	- generate final picture with the pics from the
    //	  database
    //	- save the picture in out_final_matched
    //	- BONUS: save the picture for the Bonustask in
    //	  rep_out_final_matched
    //_______________________________________________________
    imwrite( out_03_final_rep, rep_out_final_matched );
    imwrite( out_04_final, out_final_matched );
  }

  catch (exception &ex)
  {
    cout << ex.what() << endl;
    cout << "Program exited with errors!" << endl;
    return -1;
  }
  cout << "Program exited normally!" << endl;
  return 0;
}
