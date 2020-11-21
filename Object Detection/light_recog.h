/*static void on_light(int, void*)
{
Mat img_h, img_s, img_v;
const Scalar hsvgreenlw(40, 43, 46);
const Scalar hsvgreenhg(90, 255, 255);
const Scalar hsvyellowlw(20, 43, 46);
const Scalar hsvyellowhg(39, 255, 255);

tmpImg1 = frame.clone();
cvtColor(frame, hsvimg, CV_BGR2HSV);

vector<cv::Mat>hsv_vec;
split(hsvimg, hsv_vec);
img_h = hsv_vec[0];
img_s = hsv_vec[1];
img_v = hsv_vec[2];
img_h.convertTo(img_h, CV_32F);
img_s.convertTo(img_s, CV_32F);
img_v.convertTo(img_v, CV_32F);
double max_h, max_s, max_v;
minMaxIdx(img_h, 0, &max_h);
minMaxIdx(img_s, 0, &max_s);
minMaxIdx(img_v, 0, &max_v);
img_h /= max_h;
img_s /= max_s;
img_v /= max_v;
cv::Mat bw_black = img_v < 0.3;
imshow("黑色提取", bw_black);

//findobjarea(bw_black);
vector<vector<Point>>contours;
findContours(bw_black, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

if (contours.size()> 0)
{
vector<vector<Point>>::iterator iter = contours.begin();
for (; iter != contours.end();)
{
double g_conarea = contourArea(*iter);
if (g_conarea<2000)
{
iter = contours.erase(iter);
}
else
{
++iter;
}
}

vector<vector<Point>>conpoint(contours.size());
vector<Rect>boundrect(contours.size());

for (int i = 0; i < contours.size(); i++)
{
approxPolyDP(Mat(contours[i]), conpoint[i], 3, 1);
boundrect[i] = boundingRect(Mat(conpoint[i]));

if (light_shape(boundrect[i]))
{

imgroi = hsvimg(Rect(boundrect[i]));
break;
}
}


Mat lowredmat, uprredmat, tempmat;
int g = 0;
int y = 0;
int r = 0;
for (size_t y = 0; y < imgroi.rows; y++)
{
for (size_t x = 0; x < imgroi.cols; x++)
{

int H = imgroi.at<Vec3b>(y, x)[0];
int S = imgroi.at<Vec3b>(y, x)[1];
int V = imgroi.at<Vec3b>(y, x)[2];

if (S>43)
{
if (V>46)
{
if (H>10 && H<160)
{
if (H>35 && H<77)
{
++g;
}
else
{
if (H>20 && H<39)
{
++y;
}
}
}
else
{
++r;
}
}
}
}
}

if (r>g&&r > y)
{
inRange(imgroi, Scalar(0, 100, 100), Scalar(10, 255, 255), lowredmat);
inRange(imgroi, Scalar(160, 100, 100), Scalar(180, 255, 255), uprredmat);
addWeighted(lowredmat, 1, uprredmat, 1, 0, tempmat);
std::cout << "红色" << endl;
}
else
{
if (g > y&&g > r)
{
inRange(imgroi, hsvgreenlw, hsvgreenhg, tempmat);
std::cout << "绿色" << endl;
}
else
{
if (y > g&&y > r)
{
inRange(imgroi, hsvyellowlw, hsvyellowhg, tempmat);
std::cout << "黄色" << endl;
}
}
}

GaussianBlur(tempmat, tempmat, Size(2 * gauss_size + 1, 2 * gauss_size + 1), 2, 2);
Mat ele = getStructuringElement(MORPH_RECT, Size(5, 5));
morphologyEx(tempmat, tempmat, MORPH_OPEN, ele);
morphologyEx(tempmat, tempmat, MORPH_CLOSE, ele);
Canny(tempmat, tempmat, canny_lw, 2 * canny_lw, 3);

vector<vector<Point>>cons, cons1, curve, curve1;
vector<Vec4i>hierarchy;
findContours(tempmat, cons, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

if (cons.size() > 0)
{
vector<vector<Point>>::iterator it = cons.begin();
for (; it != cons.end();)
{
double g_consarea = contourArea(*it);
if (g_consarea<450)
{
it = cons.erase(it);
}
else
{
++it;
}
}

Mat pointsf;
cvtColor(imgroi, dstimg, CV_HSV2BGR);
for (int i = 0; i < cons.size(); i++)
{
Mat(cons[i]).convertTo(pointsf, CV_32F);
RotatedRect box = fitEllipse(pointsf);
if ((box.size.height)>30)
{
circle(frame, box.center, box.size.height / 2, Scalar(0, 255, 255), 8, 8, 0);
break;
}
}
}
else
{
}
}
else
{

}

}*/
/*bool light_shape(Rect(m))
{
double width = m.width;
double height = m.height;
double area = height*width;
const double shapeindex0 = 50;
const double shapeindex1 = 2;
const double shapeindex2 = 3.5;
if ((width / height<shapeindex2) && (width / height>shapeindex1))
{
if (area>shapeindex0)
{
return true;
}
else
{
return false;
}
}
}*/


#pragma once
#include<iostream>
#include<string>
#include<vector>
#include<opencv2/core/core.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

class light_recog
{
private:

	const Scalar hsvgreenlw = Scalar(40, 43, 46);
	const Scalar hsvgreenhg = Scalar(90, 255, 255);
	const Scalar hsvyellowlw = Scalar(20, 43, 46);
	const Scalar hsvyellowhg = Scalar(39, 255, 255);

public:
	Mat srcimg;
	Mat black;
	Mat imgroi;
	Mat hsvimg;
	Mat dstimg;
	
	void pre_proc(Mat &srcimg)
	{
		cvtColor(srcimg, hsvimg, CV_BGR2HSV);

		vector<cv::Mat>hsv_vec;
		Mat img_h, img_s, img_v;
		split(hsvimg, hsv_vec);
		img_h = hsv_vec[0];
		img_s = hsv_vec[1];
		img_v = hsv_vec[2];
		img_h.convertTo(img_h, CV_32F);
		img_s.convertTo(img_s, CV_32F);
		img_v.convertTo(img_v, CV_32F);
		double max_h, max_s, max_v;
		minMaxIdx(img_h, 0, &max_h);
		minMaxIdx(img_s, 0, &max_s);
		minMaxIdx(img_v, 0, &max_v);
		img_h /= max_h;
		img_s /= max_s;
		img_v /= max_v;
		black = img_v < 0.3;
	}



	void find_imgroi(Mat &black)
	{
		vector<vector<Point> >contours;
		findContours(black, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

		if (contours.size() > 0)
		{
			vector<vector<Point> >::iterator iter = contours.begin();
			for (; iter != contours.end();)
			{
				double g_conarea = contourArea(*iter);
				if (g_conarea < 5000)
				{
					iter = contours.erase(iter);
				}
				else
				{
					++iter;
				}
			}

			vector<vector<Point> >conpoint(contours.size());
			vector<Rect>boundrect(contours.size());

			for (int i = 0; i < contours.size(); i++)
			{
				approxPolyDP(Mat(contours[i]), conpoint[i], 3, 1);
				boundrect[i] = boundingRect(Mat(conpoint[i]));

				if (light_shape(boundrect[i]))
				{
					imgroi = light_recog::hsvimg(Rect(boundrect[i]));
					break;
				}
			}
		}
	}


	void color_recog(Mat &imgroi)
	{
		Mat lowredmat, uprredmat, tempmat;
		float g = 0;
		float y = 0;
		float r = 0;
		for (int y = 0; y < imgroi.rows; y++)
		{
			for (int x = 0; x < imgroi.cols; x++)
			{

				double H = imgroi.at<Vec3b>(y, x)[0];
				double S = imgroi.at<Vec3b>(y, x)[1];
				double V = imgroi.at<Vec3b>(y, x)[2];

				if (S>43)
				{
					if (V > 46)
					{
						if (H > 10 && H<160)
						{
							if (H>35 && H<77)
							{
								++g;
							}
							else
							{
								if (H>20 && H < 39)
								{
									++y;
								}
							}
						}
						else
						{
							++r;
						}
					}
				}
			}
		}

		if (r > g&&r > y)
		{
			inRange(imgroi, Scalar(0, 100, 100), Scalar(10, 255, 255), lowredmat);
			inRange(imgroi, Scalar(160, 100, 100), Scalar(180, 255, 255), uprredmat);
			addWeighted(lowredmat, 1, uprredmat, 1, 0, tempmat);
			std::cout << "红色" << endl;
		}
		else
		{
			if (g > y&&g > r)
			{
				inRange(imgroi, hsvgreenlw, hsvgreenhg, tempmat);
				std::cout << "绿色" << endl;
			}
			else
			{
				if (y > g&&y > r)
				{
					inRange(imgroi, hsvyellowlw, hsvyellowhg, tempmat);
					std::cout << "黄色" << endl;
				}
			}
		}

		GaussianBlur(tempmat, tempmat, Size(5, 5), 2, 2);
		Mat ele = getStructuringElement(MORPH_RECT, Size(5, 5));
		morphologyEx(tempmat, tempmat, MORPH_OPEN, ele);
		morphologyEx(tempmat, tempmat, MORPH_CLOSE, ele);
		Canny(tempmat, tempmat, 80, 160, 3);

		vector<vector<Point> >cons, cons1, curve, curve1;
		vector<Vec4i>hierarchy;
		findContours(tempmat, cons, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

		if (cons.size() > 0)
		{
			vector<vector<Point> >::iterator it = cons.begin();
			for (; it != cons.end();)
			{
				double g_consarea = contourArea(*it);
				if (g_consarea < 450)
				{
					it = cons.erase(it);
				}
				else
				{
					++it;
				}
			}

			Mat pointsf;
			//cvtColor(imgroi, dstimg, CV_HSV2BGR);
			for (int i = 0; i < cons.size(); i++)
			{
				Mat(cons[i]).convertTo(pointsf, CV_32F);
				RotatedRect box = fitEllipse(pointsf);
				if ((box.size.height)>30)
				{
					circle(imgroi, box.center, box.size.height / 2, Scalar(0, 255, 255), 8, 8, 0);
					break;
				}
			}
			cvtColor(imgroi, dstimg, CV_HSV2BGR);
			imshow("红绿灯", imgroi);
		}
		else
		{
		}
	}



	bool light_shape(Rect(m))
	{
		double width = m.width;
		double height = m.height;
		double area = height*width;
		const double shapeindex0 = 50;
		const double shapeindex1 = 2;
		const double shapeindex2 = 3.5;
		if ((width / height<shapeindex2) && (width / height>shapeindex1))
		{
			if (area > shapeindex0)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}



};
