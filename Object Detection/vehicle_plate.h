#include<opencv2/opencv.hpp>
#include<iostream>
#include<opencv2/core/core.hpp>
#include<opencv2/imgproc/imgproc.hpp>
using namespace cv;
using namespace std;

class vehicle_plate
{
private:

public:

	Mat getPlate(int width, int height, cv::Mat srcGray)
	{
		cv::Mat result;
		// 形态学梯度检测边缘
		morphologyEx(srcGray, result, MORPH_GRADIENT,
			Mat(1, 2, CV_8U, Scalar(1)));
		cv::imshow("1result", result);
		// 阈值化
		threshold(result, result, 255 * (0.1), 255,
			THRESH_BINARY);
		cv::imshow("2result", result);
		// 水平方向闭运算
		if (width >= 400 && width < 600)
			morphologyEx(result, result, MORPH_CLOSE,
			Mat(1, 25, CV_8U, Scalar(1)));
		else if (width >= 200 && width < 300)
			morphologyEx(result, result, MORPH_CLOSE,
			Mat(1, 20, CV_8U, Scalar(1)));
		else if (width >= 600)
			morphologyEx(result, result, MORPH_CLOSE,
			Mat(1, 28, CV_8U, Scalar(1)));
		else
			morphologyEx(result, result, MORPH_CLOSE,
			Mat(1, 15, CV_8U, Scalar(1)));
		// 垂直方向闭运算
		if (height >= 400 && height < 600)
			morphologyEx(result, result, MORPH_CLOSE,
			Mat(8, 1, CV_8U, Scalar(1)));
		else if (height >= 200 && height < 300)
			morphologyEx(result, result, MORPH_CLOSE,
			Mat(6, 1, CV_8U, Scalar(1)));
		else if (height >= 600)
			morphologyEx(result, result, MORPH_CLOSE,
			Mat(10, 1, CV_8U, Scalar(1)));
		else
			morphologyEx(result, result, MORPH_CLOSE,
			Mat(4, 1, CV_8U, Scalar(1)));
		return result;
	}
	void licence_plate(Mat in_frame, int licence)
	{
		Mat srcImage = in_frame.clone();
		//if (!srcImage.data)
		//  return 1;
		cv::Mat srcGray;
		cv::cvtColor(srcImage, srcGray, CV_RGB2GRAY);
		cv::imshow("srcGray", srcGray);
		cv::Mat result = getPlate(400, 300, srcGray);

		//中值滤波
		cv::medianBlur(srcImage, srcImage, 3);

		//转换成HSV
		cv::Mat img_h, img_s, img_v, imghsv;
		std::vector<cv::Mat> hsv_vec;
		cv::cvtColor(srcImage, imghsv, CV_BGR2HSV);
		cv::imshow("hsv", imghsv);
		//cv::waitKey(0);
		//黄色阈值化处理
		cv::Mat yellowTempMat;
		cv::inRange(imghsv, cv::Scalar(22, 100, 100), cv::Scalar(38, 255, 255), yellowTempMat);
		cv::imshow("yellowTempMat", yellowTempMat);

		// 连通域检测
		std::vector<std::vector<cv::Point> > blue_contours;
		std::vector<cv::Rect> blue_rect;
		cv::findContours(result.clone(),
			blue_contours, CV_RETR_EXTERNAL,
			CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
		// 连通域遍历 车牌目标提取
		for (size_t i = 0; i != blue_contours.size(); ++i)
		{
			cv::Rect rect = cv::boundingRect(blue_contours[i]);
			//宽高比
			double wh_ratio = double(rect.width) / rect.height;
			//计算区域非零像素
			int sub = cv::countNonZero(result(rect));
			//非零像素点占的比例
			double ratio = double(sub) / rect.area();
			//有效像素点、宽高及比例判断
			if (wh_ratio > 2 && wh_ratio < 8 && rect.height >
				12 && rect.width > 60 && ratio > 0.4)
			{
				//blue_rect.push_back(rect);
				cv::imshow("Car", srcGray(rect));
				Mat rectImage = srcGray(rect).clone();


				if (rectImage.data)
				{
					printf("yes\n");
					licence = 1;
				}
				//cv::waitKey(0);
			}
		}
		cv::imshow("result", result);
	}
};