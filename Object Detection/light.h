#include<opencv2/opencv.hpp>
#include<iostream>
#include<opencv2/core/core.hpp>
#include<opencv2/imgproc/imgproc.hpp>
using namespace cv;
using namespace std;

class light
{


private:

public:
	Mat resultImage;
	int l;

	int traffic_light(cv::Mat in_frame)
	{
		//resultImage = in_frame.clone();
		//Mat in_frame;
		//pyrDown(in_frame1, in_frame, Size(in_frame.cols / 2, in_frame.rows / 2));
		resultImage = in_frame.clone();
		//pyrDown(in_frame, resultImage, Size(in_frame.cols / 2, in_frame.rows / 2));
		//��ֵ�˲�
		medianBlur(in_frame, in_frame, 3);
		//ת����HSV��ɫ�ռ�
		Mat hsvImage;
		cvtColor(in_frame, hsvImage, cv::COLOR_BGR2HSV);
		//imshow("hsvImage", hsvImage);
		//��ɫ��ֵ������
		cv::Mat yellowTempMat;
		cv::inRange(hsvImage, cv::Scalar(22, 100, 100), cv::Scalar(38, 255, 255), yellowTempMat);
		//cv::imshow("yellowTempMat", yellowTempMat);


		//��ɫ��ֵ������
		cv::Mat greenTempMat;
		cv::inRange(hsvImage, cv::Scalar(38, 100, 100), cv::Scalar(75, 255, 255), greenTempMat);
		//cv::imshow("greenTempMat", greenTempMat);


		//��ɫ��ֵ������
		cv::Mat lowTempMat;
		cv::Mat upperTempMat;
		//����ֵ�޶�
		cv::inRange(hsvImage, cv::Scalar(0, 100, 100), cv::Scalar(10, 255, 255), lowTempMat);
		//����ֵ�޶�
		cv::inRange(hsvImage, cv::Scalar(160, 100, 100), cv::Scalar(180, 255, 255), upperTempMat);
		//cv::imshow("lowTempMat", lowTempMat);
		//cv::imshow("upperTempMat", upperTempMat);
		//��ɫ��ֵ�޶��ϲ�
		cv::Mat redTempMat;
		cv::addWeighted(lowTempMat, 1.0, upperTempMat, 1.0, 0.0, redTempMat);


		//��˹ƽ��
		cv::GaussianBlur(redTempMat, redTempMat, cv::Size(9, 9), 2, 2);
		/*Canny(redTempMat, redTempMat, 50, 100, 3);
		vector<vector<Point>>contours;
		findContours(redTempMat, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
		vector<vector<Point>>::iterator iter = contours.begin();
		for (; iter != contours.end();)
		{
			double g_conarea = contourArea(*iter);
			if (g_conarea<100)
			{
				iter = contours.erase(iter);
			}
			else
			{
				++iter;
			}
		}
		Mat pointsf;
		for (int i = 0; i < contours.size(); i++)
		{
			Mat(contours[i]).convertTo(pointsf, CV_32F);
			RotatedRect box = fitEllipse(pointsf);
			if ((box.size.height)>20)
			{
				circle(resultImage, Point(box.center.x, box.center.y), box.size.height / 2, Scalar(0, 255, 255), 10, 8, 0);
				break;
			}
		}*/
		//����任
		std::vector<cv::Vec3f> redcircles;
		cv::HoughCircles(redTempMat, redcircles, CV_HOUGH_GRADIENT, 1, redTempMat.rows, 100, 40, 10, 50);
		//��ɫԲ������ж�
		if (redcircles.size() == 0)//�ж��Ƿ��к�ɫԲ
		{
			cv::GaussianBlur(greenTempMat, greenTempMat, cv::Size(9, 9), 2, 2);
			std::vector<cv::Vec3f> greencircles;
			cv::HoughCircles(greenTempMat, greencircles, CV_HOUGH_GRADIENT, 1, greenTempMat.rows, 100, 40, 10, 50);
			if (greencircles.size() == 0)//�ж��Ƿ�����ɫ԰
			{
				cv::GaussianBlur(yellowTempMat, yellowTempMat, cv::Size(9, 9), 2, 2);
				std::vector<cv::Vec3f> yellowcircles;
				cv::HoughCircles(yellowTempMat, yellowcircles, CV_HOUGH_GRADIENT, 1, yellowTempMat.rows, 100, 40, 10, 50);
				if (yellowcircles.size() == 0)//�ж��Ƿ��л�ɫԲ
					l = 0;
				for (int k = 0; k < yellowcircles.size(); ++k)
				{
					//���Ƽ�⵽�Ļ�ɫԲ
					cv::Point center(round(yellowcircles[k][0]), round(yellowcircles[k][1]));
					int radius = round(yellowcircles[k][2]);
					cout << "\t��⵽�Ƶ�" << endl;
					cv::circle(resultImage, center, radius, cv::Scalar(255, 0, 0), 2);
				}
				//cv::imshow("Traffic_light", resultImage);
				l = 3;
			}
			for (int j = 0; j < greencircles.size(); ++j)
			{
				//���Ƽ�⵽����ɫԲ
				cv::Point center(round(greencircles[j][0]), round(greencircles[j][1]));
				int radius = round(greencircles[j][2]);
				cout << "\t��⵽�̵�" << endl;
				cv::circle(resultImage, center, radius, cv::Scalar(255, 0, 0), 2);
			}
			//cv::imshow("Traffic_light", resultImage);
			l = 2;
		}
		for (int i = 0; i < redcircles.size(); ++i)
		{
			//���Ƽ�⵽����ɫԲ
			cv::Point center(round(redcircles[i][0]), round(redcircles[i][1]));
			int radius = round(redcircles[i][2]);
			cout << "\t��⵽���" << endl;
			cv::circle(resultImage, center, radius, cv::Scalar(255, 0, 0), 2);
		}
		cv::imshow("Traffic_light", resultImage);
		l = 1;
		return l;
	}

	bool light_valid(std::vector<cv::Vec3f> circles)
	{

	}

};