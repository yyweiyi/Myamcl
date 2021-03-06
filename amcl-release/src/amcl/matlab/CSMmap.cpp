#include "CSMmap.h"
#include <math.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

using namespace Eigen;
using namespace cv;

namespace amcl
{

double normpdf(double dist, double mu, double sigma)
{
    return exp(-(dist - mu) * (dist - mu) / (2 * sigma * sigma)) / (sqrt(2 * 3.14) * sigma);
}

LoadMap::LoadMap(const nav_msgs::OccupancyGrid &map_msg)
{
    width_ = map_msg.info.width;
    height_ = map_msg.info.height;
    res_ = map_msg.info.resolution;
    origin_x = map_msg.info.origin.position.x;
    origin_y = map_msg.info.origin.position.y;

    hit_ = MatrixXd::Zero(height_, width_); // Col major , not row major

	Mat imageOri;
	imageOri = imread("/home/liqing/bagfiles/map/map.pgm",IMREAD_GRAYSCALE);
	//imageOri = imread("/home/liqing/mapLoad.png",IMREAD_GRAYSCALE);
	//imwrite("/home/liqing/mapLoad.png",imageOri);

	double occ_th = 0.51; //3 state (hit : <=10 , unknow : ==50 , free : >= 180)
	double free_th = 0.49;
	double occ = 0;
	unsigned char* p;
	for (size_t x = 0; x < imageOri.rows; ++x) 
	{
		p = imageOri.ptr<uchar>(x);
		for (size_t y = 0; y < imageOri.cols; ++y) 
		{
		   	occ = (255 - *(p+y)) / 255.0 ;
		   	if(occ > occ_th)
				hit_(x, y) = 1;
			else if(occ < free_th)
				hit_(x,y) = -1;
			else
				hit_(x,y) = 0;
	    }
	}
	
    loadmap_ = MatrixXd::Zero(width_, height_); // swap width and height
    MatrixXd positivemap = MatrixXd::Zero(height_, width_);
    MatrixXd tempPositivemap = MatrixXd::Zero(height_, width_);

    //Gaussian filter
    int windowSize = 3;
    double sigma = sqrt(2) * windowSize * res_ / 3;
    MatrixXd gaussianWindow = MatrixXd::Zero(2 * windowSize + 1, 2 * windowSize + 1);
    double max_prob = normpdf(0, 0, sigma) * 5;
    for (int i = -windowSize; i < windowSize + 1; i++)
    {
		for (int j = -windowSize; j < windowSize + 1; j++)
		{
			double dist = res_ * sqrt(i * i + j * j);
			gaussianWindow(i + windowSize, j + windowSize) = normpdf(dist, 0, sigma) / max_prob;
		}
    }


    for (int y = 0; y < height_; y++)
    {
		for (int x = 0; x < width_; x++)
		{
			if (hit_(y, x) == 1)
			{
				for (int i = -windowSize; i < windowSize + 1; i++)
				{
					for (int j = -windowSize; j < windowSize + 1; j++)
					{
						bool inmap = (y + i > -1) && (y + i < height_) && (x + j > -1) && (x + j < width_);
						if (inmap)
						{
							double p = positivemap(y + i, x + j);
							double q = gaussianWindow(i + windowSize, j + windowSize);
							positivemap(y + i, x + j) = (p + q) > 1 ? 1 : (p + q); //positivemap is same as Hmap.lookup original
						}
					}
				}
			}
		}
    }

    for (size_t y = 0; y < height_; ++y)
    {
		for (size_t x = 0; x < width_; ++x)
		{
			double data = positivemap(y, x);
			tempPositivemap(height_ - y - 1, x) = data;
		}
    }

    loadmap_ = tempPositivemap.transpose();
       Mat image(height_,width_,CV_8U,Scalar(128));
       unsigned char* p1;

       for (size_t y = 0; y < height_; ++y) {
         p1 = image.ptr<uchar>(y);
         for (size_t x = 0; x < width_; ++x) {
           const size_t i = x + (height_ - y - 1) * width_;
           int data = positivemap(y,x)*255;
           *(p1+x) = data;

          }
         }
        imshow("debug",image);
        imwrite("/home/liqing/map.png",image);
}

LoadMap::LoadMap()
{
	std::cout << "LoadMap::LoadMap()" << std::endl;     
	res_ = 0.05;
    origin_x = -19.47783432006836;
    origin_y = -5.01034851074219;


	Mat imageOri;
	imageOri = imread("/home/liqing/bagfiles/map/map.pgm",IMREAD_GRAYSCALE);

    width_ = imageOri.cols;
    height_ = imageOri.rows;
	hit_ = MatrixXd::Zero(height_, width_); // Col major , not row major
	
	double occ_th = 0.51;
	double free_th = 0.49;
	double occ = 0;
	unsigned char* p;
	for (size_t x = 0; x < imageOri.rows; ++x) 
	{
		p = imageOri.ptr<uchar>(x);
		for (size_t y = 0; y < imageOri.cols; ++y) 
		{
		   	occ = (255 - *(p+y)) / 255.0 ;
		   	if(occ > occ_th)
				hit_(x, y) = 1;
			else if(occ < free_th)
				hit_(x,y) = -1;
			else
				hit_(x,y) = 0;
	    }
	}
	
    loadmap_ = MatrixXd::Zero(width_, height_); // swap width and height
    MatrixXd positivemap = MatrixXd::Zero(height_, width_);
    MatrixXd tempPositivemap = MatrixXd::Zero(height_, width_);

    //Gaussian filter
    int windowSize = 3;
    double sigma = sqrt(2) * windowSize * res_ / 3;
    MatrixXd gaussianWindow = MatrixXd::Zero(2 * windowSize + 1, 2 * windowSize + 1);
    double max_prob = normpdf(0, 0, sigma) * 5;
    for (int i = -windowSize; i < windowSize + 1; i++)
    {
		for (int j = -windowSize; j < windowSize + 1; j++)
		{
			double dist = res_ * sqrt(i * i + j * j);
			gaussianWindow(i + windowSize, j + windowSize) = normpdf(dist, 0, sigma) / max_prob;
		}
    }


    for (int y = 0; y < height_; y++)
    {
		for (int x = 0; x < width_; x++)
		{
			if (hit_(y, x) == 1)
			{
				for (int i = -windowSize; i < windowSize + 1; i++)
				{
					for (int j = -windowSize; j < windowSize + 1; j++)
					{
						bool inmap = (y + i > -1) && (y + i < height_) && (x + j > -1) && (x + j < width_);
						if (inmap)
						{
							double p = positivemap(y + i, x + j);
							double q = gaussianWindow(i + windowSize, j + windowSize);
							positivemap(y + i, x + j) = (p + q) > 1 ? 1 : (p + q); //positivemap is same as Hmap.lookup original
						}
					}
				}
			}
		}
    }

    for (size_t y = 0; y < height_; ++y)
    {
		for (size_t x = 0; x < width_; ++x)
		{
			double data = positivemap(y, x);
			tempPositivemap(height_ - y - 1, x) = data;
		}
    }

    loadmap_ = tempPositivemap.transpose();
       Mat image(height_,width_,CV_8U,Scalar(128));
       unsigned char* p1;

       for (size_t y = 0; y < height_; ++y) {
         p1 = image.ptr<uchar>(y);
         for (size_t x = 0; x < width_; ++x) {
           const size_t i = x + (height_ - y - 1) * width_;
           int data = positivemap(y,x)*255;
           *(p1+x) = data;

          }
         }
        //imshow("debug",image);
        imwrite("/home/liqing/map_331.png",image);
}



LoadMap::~LoadMap()
{
}
}
