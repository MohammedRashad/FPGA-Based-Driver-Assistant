#include <stdlib.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <string.h>
#include <fstream>
#include <opencv2/ml/ml.hpp>
#include <stdio.h>
#include "detectTrafficLight.h"
using namespace std;
using namespace cv;
int threshold_value = 150;
int main()
{
	//Mat img = imread("/home/cube26/shruti/signal/data/ko.jpg");
		//Mat img = imread("/home/cube26/shruti/signal/data/c.jpg");

	//Mat img = imread("/home/cube26/shruti/signal/data/f.jpg");
	//	Mat img = imread("/home/cube26/shruti/signal/data/signal.jpg");

			//Mat img = imread("/home/cube26/shruti/signal/data/nightligh.jpg");

	    VideoCapture cap(1);
	    Mat img;
	    initDetector();

do
{
	cap>>img;
Size s(640,480);
resize(img,img,s);
	Mat imgHSV, imgGRAY;

	cvtColor(img, imgGRAY, CV_BGR2GRAY);

	Mat probable;
	Rect overhead ;
	overhead.x = 0;
	overhead.y = 0;
	overhead.width = imgGRAY.cols;
	overhead.height = imgGRAY.rows/2;
	probable = imgGRAY(overhead);

	//imshow("req area ", probable);

	int min_len = 10;
	int max_len = 70;
	int min_ht = 39;
	int max_ht = 100;

	int x=1;

	//waitKey(0);
     MSER mser/*(5,625,6400,0.5,0.4)*/;
      vector<vector<Point> > mser_points;
      Mat newimg=img.clone() ;
      mser(probable, mser_points, Mat());
       for (int i = 0; i < mser_points.size(); i++)
    	{  Rect r = boundingRect(mser_points[i]);
    		
    		if((r.width>min_len && r.width<max_len) && (r.height>min_ht && r.height<max_ht))
    		{
    		//newimg=img.clone();
    		Mat detect = newimg(r);
    		Mat check = detect.clone();
    		Size s2(48,96);
    		resize(check,check,s2);
    		int result = detectTrafficLight(check);
    		if(result==1)
		    		{//rectangle(newimg, r , Scalar(0,255,0),2);
		    			cout<<"****YES********";
		    		cvtColor(detect, detect, CV_BGR2YCrCb);
		    		//imshow("YCbCr",detect);
		    		vector<Mat> ycrcbchannels(3);
		    		split(detect, ycrcbchannels);
		    		//Mat split_mat = Mat::zeros(Size(detect.cols, detect.rows), CV_8UC1); 
		    		Mat roi_cr = ycrcbchannels[1];
		    		Mat roi_cb = ycrcbchannels[2];
		    		
		    			 Mat roi_binary_cr;
		    			 Mat roi_binary_cb;
		    			
		    		  GaussianBlur( roi_cr, roi_binary_cr, Size(9, 9), 2, 2 );
		    		  GaussianBlur(roi_cb, roi_binary_cb, Size(9,9), 2,2 );
		    		  threshold(roi_binary_cr, roi_binary_cr, 150, 255, THRESH_BINARY);
		      			threshold(roi_binary_cb, roi_binary_cb, 150,255,THRESH_BINARY);
		    		 // imshow("binary roi", roi_binary);
		    		 // imshow("cr",roi_binary_cr);
		    		 // waitKey(10000);
		    			//imshow("cb",roi_binary_cb);

		    			Moments cbMoments = moments(roi_binary_cb);
		    			Moments crMoments = moments(roi_binary_cr);

		    			 double redArea = crMoments.m00;
		    			  double greenArea = cbMoments.m00;


						      vector<vector<Point> > contoursR;
						        vector<vector<Point> > contoursG;
		  						vector<Vec4i> hierarchy1;
		  						vector<Vec4i> hierarchy2;

						       /// Find contours
						  findContours( roi_binary_cr.clone(), contoursR, hierarchy1, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
						  findContours( roi_binary_cb.clone(), contoursG, hierarchy2, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

						  /// Draw contours
						  Mat drawingRED = Mat::zeros( roi_binary_cr.size(), CV_8UC3 );
						  for( int i = 0; i< contoursR.size(); i++ )
						     {
						       Scalar color = Scalar( 0,0,255 );
						       drawContours( drawingRED, contoursR, i, color, 2, 8, hierarchy1, 0, Point() );
						     }

						   Mat drawingGREEN = Mat::zeros( roi_binary_cb.size(), CV_8UC3 );
						  for( int i = 0; i< contoursG.size(); i++ )
						     {
						       Scalar color = Scalar( 0,255,0 );
						       drawContours( drawingGREEN, contoursG, i, color, 2, 8, hierarchy2, 0, Point() );

						     }
						     Rect red, green,test;
						     float aspectRED, aspectGREEN;
						     float aspect;
						     float area;
						     //std::vector<Rect> v;
						    	cout<<"\nred\n";
						    for(int i=0;i<contoursR.size();i++)
						     {
						     	 test = boundingRect(contoursR[i]);
						     	 aspect = (float)test.height/test.width;
						     	 area = test.height*test.width;
						     	 cout<<aspect<<"--";
						     	 if((aspect>0.7 && aspect<1.3) && (area>40 && area<150) )
						     	 {red=test;
						     	 aspectRED=aspect;
						     	 cout<<"**area: "<<area<<" **";
						     	rectangle(drawingRED,red,Scalar(255,255,0));
						     	rectangle(img, r , Scalar(0,0,255),2);
		    			  			cout<<"\nRED LIGHT";
						    	}
						    	//imshow("red",drawingRED);
						    	//waitKey(0);
						     }
						     cout<<"\ngreen\n";
						     for(int i=0;i<contoursG.size();i++)
						     {
						     	test = boundingRect(contoursG[i]);
						     	aspect = (float)test.height/test.width;
						     	area = test.height*test.width;

						     	cout<<aspect<<"--";

						     	 if(aspect>0.7 && aspect<1.3 && (area>40 && area<130))
						     	 {green=test;
						     	 aspectGREEN=aspect;
						       	 cout<<"**area: "<<area<<" **";

						     	rectangle(drawingGREEN,green,Scalar(255,0,255));
						     	rectangle(img,r,Scalar(0,255,0),2);
		    			  			cout<<"\nGREEN LIGHT";
					     		}
					     	//	imshow("green",drawingGREEN);
					     	//	waitKey(0);
						     }

				     }
				     
				     else cout<<"****NO***";
				 //  imshow("circle", drawingRED);
				  // imshow("circle", drawingGREEN);
 

    			  /*if((redArea!=0  || greenArea!=0 ) && (contoursG.size()==1 || contoursR.size()==1))
    			  	{
    			  		if(contoursR.size()>0 && (aspectRED>0.8 && aspectRED<1.2) )
    			  		{
    			  			rectangle(img, r , Scalar(255,0,0),2);
    			  			cout<<"\nRED LIGHT";
    			  		}

    			  		else if(contoursG.size()>0 && (aspectGREEN>0.8 && aspectGREEN<1.2) )
    			  		{	rectangle(img,r,Scalar(255,0,0),2);
    			  			cout<<"\nGREEN LIGHT";
    			  		}
    			  		imshow("roi",detect);
    			 // 	cout<<"\n--"<<contoursG.size()<<"--"<<contoursR.size();
    			  		imshow("cr",roi_binary_cr);
    		 			waitKey(10000);
    					imshow("cb",roi_binary_cb);
    					waitKey(10000);
    					imshow("cr",drawingRED);
    		 			waitKey(10000);
    					imshow("cb",drawingGREEN);
    					waitKey(10000);
    					 /*cout<<"\n"<<(int)redArea<<"------"<<(int)greenArea;
    					 cout<<"*****"<<red_area_ratio;
    					 cout<< "$$$$$$$$ "<<green_area_ratio;*/
//    				}

				
    			
    	
    		//cout<<"\n"<<r.height<<" "<<r.width;*/
    	}
	    }
	  imshow("yo",img);
               waitKey(1);

}while(cap.grab());
cap.release();


	
}