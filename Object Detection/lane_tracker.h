#pragma once
#include<opencv2/opencv.hpp>
#include<iostream>
#include<opencv2/core/core.hpp>
#include<opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

class lane_tracker
{
private:
	cv::Mat srcimg;
	std::vector<cv::Vec4i>lines;
	int minvote;
	int thresh;
	double minlength;
	double maxgap;
	int shift;
	int gssize;


public:
	lane_tracker() :thresh(50), minlength(80), maxgap(30), gssize(2), minvote(100){}
	Mat cannyimg, imgroi;
	int x0 = 235, y0 = 250;


	void setpara(int minv, int minl,int maxg,int ss,int thre,Mat src)
	{
		minvote = minv;
		minlength = minl;
		maxgap = maxg;
		gssize = ss;
		thresh = thre;
		/*createTrackbar("", "", &minvote, 80, on_lane);
		createTrackbar("", "", &minlength, 150, on_lane);
		createTrackbar("", "", &maxgap, 50, on_lane);
		createTrackbar("", "", &thresh, 120, on_lane);
		createTrackbar("", "", &ss, 150, on_lane);*/
	}

	void on_lane(Mat &frame)
	{
		Mat grayimg, foreground;
		lines.clear();
		//halfimg = frame(Rect(0, frame.rows / 2, frame.cols, frame.rows / 2));
		cvtColor(frame, grayimg, CV_BGR2GRAY);
		//grayimg = grayimg(Rect(0, grayimg.rows / 2, grayimg.cols, grayimg.rows / 2));
		GaussianBlur(grayimg, foreground, Size(gssize * 2 + 1, gssize * 2 + 1), 0);
		Canny(foreground, cannyimg, thresh, 2 * thresh, 3);
		//imshow("canny", cannyimg);
		HoughLinesP(cannyimg, lines, 1, CV_PI / 180, minvote, minlength, maxgap);
		vector<Vec4i>::const_iterator it = lines.begin();

		vector<Vec4i>lanel, laner;
		while (it!=lines.end())
		{
			if ((*it)[0]==(*it)[2])
			{
			}
			else
			{
				float dy = ((*it)[3] - (*it)[1]);
				float dx = ((*it)[2] - (*it)[0]);
				double k = dy / dx;
				int C = (*it)[2]*(*it)[1] - (*it)[0]*(*it)[3];
				double dis = abs(dy*x0 + dx*y0 + C) / sqrt(dy*dy + dx*dx);
				dis = 10 * dis;

				


				if (fabs(k)>0.5)
				{
					int midx = ((*it)[0] + (*it)[2]) / 2;
					string direct;
					if (midx<frame.cols/2)
					{
						lanel.push_back((*it));
						Point pt1 = Point((*it)[0], (*it)[1]);
						Point pt2 = Point((*it)[2], (*it)[3]);
						line(frame, pt1, pt2, Scalar(255, 255, 0), 2);
						direct = "×ó";
					}
					else
					{
						laner.push_back((*it));
						Point pt1 = Point((*it)[0], (*it)[1]);
						Point pt2 = Point((*it)[2], (*it)[3]);
						line(frame, pt1, pt2, Scalar(0, 255, 255), 2);
						direct = "ÓÒ";
					}
					
					cout << "Ð±ÂÊ= " << k << "   " << "¾àÀë" << direct << "³µµÀÏß¾àÀë= " << dis << endl;
				}
			}
			++it;
		}

	}

	/*std::vector<cv::Vec4i> removelines(const cv::Mat &orientations, double percentage, double delta)
	{
		std::vector<cv::Vec4i>::iterator it = lines.begin();

		while (it != lines.end())
		{
			int x1 = (*it)[0];
			int x2 = (*it)[2];
			int y1 = (*it)[1];
			int y2 = (*it)[3];

			double ori1 = atan2(static_cast<double>(y1 - y2), static_cast<double>(x1 - x2));
			if (ori1>CV_PI)
			{
				ori1 = ori1 - 2 * CV_PI;

			}
			double ori2 = atan2(static_cast<double>(y2 - y1), static_cast<double>(x2 - x1));
			if (ori2>CV_PI)
			{
				ori2 = ori2 - 2 * CV_PI;
			}

			cv::LineIterator lit(const cv::Mat &orientations, cv::Point(x1, y1), cv::Point(x2, y2))
			{
				int i, count = 0;
				for (i = 0, count = 0; i < lit.count; i++, ++lit)
				{
					float ori = *(reinterpret_cast<float*>(*lit));

					if (std::min(fabs(ori - ori1), fabs(ori - ori2))<delta)
					{
						cout++;
					}

				}
				double consistency = count / static_cast<double>(i);

				if (cosistency<percentage)
				{
					(*it)[0] = (*it)[1] = (*it)[2] = (*it)[3];
				}
				++it;

			}
			return lines;

		}
	}*/
};