#pragma once
#include<iostream>
#include<string>
#include<vector>
#include<opencv2/core/core.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

class car_plate{

private:
	Mat grayimg, blurimg, result;

public:
	Mat src;

	void findplate(Mat &frame)
	{
		cvtColor(frame, grayimg, CV_BGR2GRAY, 0);
		blur(grayimg, blurimg, Size(3, 3));
		morphologyEx(blurimg, result, MORPH_GRADIENT, Mat(1, 2, CV_8U, Scalar(1)));
		morphologyEx(result, result, MORPH_CLOSE, Mat(1, 25, CV_8U, Scalar(1)));
		vector<vector<Point>> cons;
		findContours(result, cons, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
		//vector<vector<Rect>> boundrect(cons.size());
		Point2f vertices[4];
		for (size_t i = 0; i < cons.size(); i++)
		{
			RotatedRect box = minAreaRect(cons[i]);
			box.points(vertices);
			Rect brect = box.boundingRect();
			int sub = countNonZero(result(brect));
			double ratio = sub / box.size.area();
			double wh = box.size.width / box.size.height;
			if (box.size.height>12&&box.size.width>60&&ratio>0.7&&wh>2&&wh<5)
			{
				for (size_t i = 0; i < 4; i++)
				{
					line(frame, vertices[i], vertices[(i + 1) % 4], Scalar(0, 255, 0), 5);
				}

				break;
			}
		}
	}


};