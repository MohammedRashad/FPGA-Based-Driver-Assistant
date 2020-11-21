#include <SDKDDKVer.h>
#include <stdio.h>
#include <tchar.h>
#include<iostream>
#include<opencv2/opencv.hpp>
#include<opencv2/video/background_segm.hpp>

using namespace cv;
using namespace std;

class car{

public:

	static bool descSort(vector<Point> p1, vector<Point> p2) {
		return contourArea(p1) > contourArea(p2);
	}


	void findcar(Mat &frame, BackgroundSubtractorMOG2 &a) 
	{

		Mat mask, srcImage;

		//Ptr<BackgroundSubtractorMOG2> bgsubtractor = creatBackgroundSubtractorMOG2();
		//BackgroundSubtractorMOG2 bgsubtractor(500, 16, 0);
		//bgsubtractor->setVarThreshold(20);

		//for (int  k = 0; k < 100; k++)
		//{
		//    //读取当前帧
		//    capture >> frame;

		//    if (frame.empty())
		//    {
		//        break;
		//    }
		// bgsubtractor->apply(frame, mask, 0.2);
		//}


		a(frame, mask, -1);

		//cvtColor(mask, mask, COLOR_GRAY2BGR);

		medianBlur(mask, mask, 5);
		//morphologyEx(mask, mask, MORPH_DILATE, getStructuringElement(MORPH_RECT, Size(5, 5)));

		morphologyEx(mask, mask, MORPH_OPEN, getStructuringElement(MORPH_RECT, Size(5, 5)));

		morphologyEx(mask, mask, MORPH_CLOSE, getStructuringElement(MORPH_RECT, Size(5, 5)));

		//imshow("混合高斯建模", mask);
		//waitKey(30);


		mask.copyTo(srcImage);
		vector<vector<Point>> contours, cons(3);

		findContours(srcImage, contours, RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);


		//imshow("轮廓获取", srcImage);

		//if (contours.size() < 1) continue;

		Rect rct;
	    if (contours.size() > 1)
		{

			/*if (contours.size()>=4)
			{
				cons[0] = contours[0];
				cons[1] = contours[1];
				cons[2] = contours[2];
			}
			else
			{
				cons = contours;
			}*/
			sort(contours.begin(), contours.end(), car::descSort);

			int m = 0;
			for (int i = 0; i < cons.size(); i++)
			{
				
				if (contourArea(contours[i]) < contourArea(contours[0])*0.6 || contourArea(contours[i])<600)
					continue;
				m = m + 1;
				rct = boundingRect(contours[i]);
				rectangle(frame, rct, Scalar(0, 255, 0), 2);

			}
			cout << "检测到" << m << "辆车" << endl;

		}

	}
};