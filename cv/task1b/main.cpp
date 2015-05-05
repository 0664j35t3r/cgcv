#include <iostream>
#include <iomanip>
#include <algorithm>

#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <vector>
#include <cmath>

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
  int hist_w = 512;
  int hist_h = 400;
  int bin_w = cvRound( ( double )hist_w / input.cols );
  Mat histImage( hist_h, hist_w, CV_8UC3, Scalar( 0, 0, 0 ) );
  Mat hist = input.clone();
  normalize(hist, hist, 0, 255, NORM_MINMAX);

  /// Draw for each channel
  for (int i = 1; i < hist.cols; i++)
  {
    line( histImage, Point( bin_w * ( i - 1 ), hist_h - cvRound( hist.at<float>( i - 1 )) ),
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
  Mat hist = Mat::zeros(1, 256, CV_32FC1);

  for(int iter_y = 0; iter_y < lab.rows; iter_y++)
    for(int iter_x = 0; iter_x < lab.cols; iter_x++)
       hist.at<float>( lab.at<uchar>(iter_y, iter_x) )++;
  
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
//=============================================================
void calculateCdf(Mat& cdf_zero, Mat& hist)
{
  float sum_cdf = 0;
  for (int i_color = 0; i_color < 256; i_color++)
  {
    //                                                 y , x
    sum_cdf += hist.at<float>(0, i_color);
    cdf_zero.at<float>(0, i_color) = sum_cdf;
  }
}

void calculateLut(vector<uchar>& lut_scr, Mat src_cdf, Mat dst_cdf )
{
  int result;
  float cur_abs;
  float prev_abs;
  
  for (int i = 0; i < 256; i++)
  {
    result = 0;
    prev_abs = abs(dst_cdf.at<float>(i) - src_cdf.at<float>(0));
    for (int j = 0; j < 256; j++)
    {
      cur_abs = abs(dst_cdf.at<float>(i) - src_cdf.at<float>(j));
      if (prev_abs > cur_abs)
      {
        result = j;
        prev_abs = cur_abs;
      }
      else if(prev_abs == cur_abs)
      {
        if(abs(result - i) > abs(j - i))
        {
          result = j;
          prev_abs = cur_abs;
        }
      }
    }
    lut_scr.push_back(result);
  }
}

void matchHistograms( Mat patch_src, Mat patch_dst, Mat& matched )
{
  matched = Mat::zeros( patch_src.size(), CV_32FC1 );
  
  Mat src_lab = patch_src.clone();
  Mat dst_lab = patch_dst.clone();
  
  cvtColor( src_lab, src_lab, CV_BGR2Lab );
  cvtColor( dst_lab, dst_lab, CV_BGR2Lab );
  vector<Mat> channel_src(3);
  vector<Mat> channel_dst(3);
  split( src_lab, channel_src );
  split( dst_lab, channel_dst );
  
  Mat src_hist_a = getHistogram( channel_src[1] );
  Mat src_hist_b = getHistogram( channel_src[2] );
  Mat dst_hist_a = getHistogram( channel_dst[1] );
  Mat dst_hist_b = getHistogram( channel_dst[2] );
  
  normalize(src_hist_a, src_hist_a, 1, 256, NORM_L1);
  normalize(src_hist_b, src_hist_b, 1, 256, NORM_L1);
  normalize(dst_hist_a, dst_hist_a, 1, 256, NORM_L1);
  normalize(dst_hist_b, dst_hist_b, 1, 256, NORM_L1);
  
  // calculate cdf
  Mat src_cdf_a = Mat::zeros( 1, 256, CV_32FC1 );
  Mat src_cdf_b = Mat::zeros( 1, 256, CV_32FC1 );
  Mat dst_cdf_a = Mat::zeros( 1, 256, CV_32FC1 );
  Mat dst_cdf_b = Mat::zeros( 1, 256, CV_32FC1 );
  
  calculateCdf(src_cdf_a, src_hist_a);
  calculateCdf(dst_cdf_a, dst_hist_a);
  calculateCdf(src_cdf_b, src_hist_b);
  calculateCdf(dst_cdf_b, dst_hist_b);

  // calculate lut
  vector<uchar> lut_scr_a;
  vector<uchar> lut_scr_b;
  calculateLut(lut_scr_a, src_cdf_a, dst_cdf_a);
  calculateLut(lut_scr_b, src_cdf_b, dst_cdf_b);
  
  // write it in every patch
  for (int y = 0; y < patch_src.rows; y++)
  {
    for (int x = 0; x < patch_src.cols; x++)
    {
      channel_dst[1].at<uchar>(y, x) = lut_scr_a[channel_dst[1].at<uchar>(y, x)];
      channel_dst[2].at<uchar>(y, x) = lut_scr_b[channel_dst[2].at<uchar>(y, x)];
    }
  }
  
  merge( channel_dst, matched );
  cv::cvtColor( matched, matched, CV_Lab2BGR );
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
  // create one new patch
  Mat tmp = Mat::zeros( patch_size, patch_size, CV_32FC1 );

  // write the pixels in this patch 
  for (int y = 0; y < image.rows; y = y + patch_size)
  {
    for (int x = 0; x < image.cols; x = x + patch_size)
    {
      tmp = image.colRange(x, x + patch_size).rowRange(y, y + patch_size);
      patches.push_back(tmp);
    }
  } 
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
//==============================================================
void setPatches( Mat &image, vector<Mat> &patches, unsigned int patch_size )
{
  unsigned patch_count_y = image.rows / patch_size;
  unsigned patch_count_x = image.cols / patch_size;
  
  for (int y_patch = 0; y_patch < patch_count_y; y_patch++)
  {
    for (int x_patch = 0; x_patch < patch_count_x; x_patch++)
    {
      for (int y_pixel = 0; y_pixel < patch_size; y_pixel++)
      {
        for (int x_pixel = 0; x_pixel < patch_size; x_pixel++)
        {
          image.at<Vec3b>(y_patch * patch_size + y_pixel, x_patch * patch_size + x_pixel) = 
            patches.at(x_patch + y_patch * patch_count_x).at<Vec3b>(y_pixel, x_pixel);
        }
      }
    }
  } 
} 


//==============================================================
// retrieveHistogramsFromFile( string file_name )
//------------------------------------------------------------------------------
// TODO:
//	- Nothing!
//	- Do not change anything here
//==============================================================
Mat retrieveHistogramsFromFile( string file_name )
{
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
  return returnValue;
}

int h_hist_max = -INT_MAX;
int h_hist_min =  INT_MAX;
int s_hist_max = -INT_MAX;
int s_hist_min =  INT_MAX;
int v_hist_max = -INT_MAX;
int v_hist_min =  INT_MAX;
int max_idx =    -INT_MAX;
int min_idx =     INT_MAX;

float bin_max = 0.0f;

//==============================================================
// getHSVHistogram(const Mat &hsv)
//------------------------------------------------------------------------------
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
//==============================================================
Mat getHSVHistogram( const Mat &hsv )
{
  Mat hist = Mat::zeros( 1, DIMENSIONS, CV_32FC1 );
  
  // iterate over the orginal image, which will be converted into the HSV space!
  for (int y_pixel = 0; y_pixel < hsv.rows; y_pixel++)
  {
    for (int x_pixel = 0; x_pixel < hsv.cols; x_pixel++)
    {
      // calculate hsv values
      Vec3b pixel = hsv.at<Vec3b>(y_pixel, x_pixel); 
      uchar h = 0;
      if(pixel[0] < 180)
        h = floor(pixel[0] / 10);
      
      uchar s = floor(pixel[1] / 86);
      uchar v = floor(pixel[2] / 86);
      
      // index(i) = 9h(i)+3s(i)+v(i) increased
      hist.at<float>(0, 9 * h + 3 * s + v)++;
    }
  }
  return hist;
}

//==============================================================
// main()
//
//==============================================================
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
    vector<Mat> vecPatches;
    
    for(int i_mosaic = 1; i_mosaic <= num_images; i_mosaic++)
    {
      const std::string mosaic_peaces_name = mosaic_path + to_string(i_mosaic) + PNG;
      mosaic_pieces.push_back( imread(mosaic_peaces_name) ); 
    }
    
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
    Mat mDBHistograms = retrieveHistogramsFromFile( mosaic_histo ); // database_histo
    
    cvtColor(mQueryImage, mQueryImage, CV_BGR2HSV);
    imwrite( out_01_hsv, mQueryImage );
    
    getPatches(mQueryImage, vecPatches, mos_size);
    
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
    Mat mInputHistograms = Mat::zeros( 0 , DIMENSIONS, CV_32FC1 ); 
    Mat out_mInputHistograms = Mat::zeros( height, DIMENSIONS, CV_32FC1 );

    for(auto &it_patch : vecPatches)
      mInputHistograms.push_back( getHSVHistogram(it_patch) );

    imwrite( out_02_hist, mInputHistograms );
    
    //_______________________________________________________
    // TODO:
    //  - set KD-Tree:
    //      use the flann lib
    //      for the distance use FLANN_DIST_EUCLIDEAN
    //_______________________________________________________
    flann::Index flann_index( mDBHistograms, 
                              cv::flann::KDTreeIndexParams(KDTreeIPara), 
                              cvflann::FLANN_DIST_EUCLIDEAN );
    
    vector<Mat> vecPatches_matched;
    vector<Mat> rep_vecPatches_matched;

    vector<int> indices;
    vector<float> dists;
    for(int i_intialize = 0; i_intialize < 5; i_intialize++)
    {
      indices.push_back(0);
      dists.push_back(0);
    }
    
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
    flann::SearchParams search_params(KDTreeSearchParams);
    Mat match = Mat::zeros( mQueryImage.size(), CV_8UC3 );
    deque<int> pic_counter;
    for(auto &it_patch : vecPatches)
    {
      // Source:  http://stackoverflow.com/questions/10336568/how-to-use-opencv-flannindex
      // query, indices, dists, number of nearest neighbour, params
      Mat patch = getHSVHistogram(it_patch);
      flann_index.knnSearch( patch, indices, dists, 5, search_params );
      
      // to be sure it is hsv
      cvtColor(it_patch, it_patch, CV_HSV2BGR);
      matchHistograms(it_patch, mosaic_pieces.at(indices.front()), match);
      vecPatches_matched.push_back( match );
      
      
      // reputation
      if( !pic_counter.empty() )
      {
        if( std::find( pic_counter.begin(), pic_counter.end(), indices.at(0)) == pic_counter.end())
          pic_counter.push_back( indices.at(0) );
        if( std::find( pic_counter.begin(), pic_counter.end(), indices.at(1)) == pic_counter.end())
          pic_counter.push_back( indices.at(1) );
        if( std::find( pic_counter.begin(), pic_counter.end(), indices.at(2)) == pic_counter.end())
          pic_counter.push_back( indices.at(2) );
        if( std::find( pic_counter.begin(), pic_counter.end(), indices.at(3)) == pic_counter.end())
          pic_counter.push_back( indices.at(3) );
        if( std::find( pic_counter.begin(), pic_counter.end(), indices.at(4)) == pic_counter.end())
          pic_counter.push_back( indices.at(4) );
      }
      else
      {
        pic_counter.push_back(indices.at(0));
        pic_counter.push_back(indices.at(1));
        pic_counter.push_back(indices.at(2));
        pic_counter.push_back(indices.at(3));
        pic_counter.push_back(indices.at(4));
      }
      
      int pos = std::find( pic_counter.begin(), pic_counter.end(), indices.at(0)) - pic_counter.begin();
      int tmp = std::find( pic_counter.begin(), pic_counter.end(), indices.at(1)) - pic_counter.begin();
      if(tmp > pos)
        pos = tmp;
      tmp = std::find( pic_counter.begin(), pic_counter.end(), indices.at(2)) - pic_counter.begin();
      if(tmp > pos)
          pos = tmp;
      tmp = std::find( pic_counter.begin(), pic_counter.end(), indices.at(3)) - pic_counter.begin();
      if(tmp > pos)
        pos = tmp;
      tmp = std::find( pic_counter.begin(), pic_counter.end(), indices.at(4)) - pic_counter.begin();
      if(tmp > pos)
        pos = tmp;
      
      cout << pos << endl;
      
      matchHistograms(it_patch, mosaic_pieces.at(pic_counter.at(pos)), match);
      pic_counter.push_front(pic_counter.at(pos));
      pic_counter.erase(pic_counter.begin() + pic_counter[pos + 1]);
      rep_vecPatches_matched.push_back(match);
    }

    //_______________________________________________________
    // TODO:
    //	- generate final picture with the pics from the
    //	  database
    //	- save the picture in out_final_matched
    //	- BONUS: save the picture for the Bonustask in
    //	  rep_out_final_matched
    //_______________________________________________________
    setPatches(out_final_matched, vecPatches_matched, mos_size);
    setPatches(rep_out_final_matched, rep_vecPatches_matched, mos_size);
    
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
