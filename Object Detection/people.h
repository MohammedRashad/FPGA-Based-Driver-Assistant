#include<iostream>
#include<string>
#include<vector>
#include<opencv2/core/core.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/objdetect/objdetect.hpp>

using namespace cv;
using namespace std;

class people{
private:


public:

	Mat src1, src2;


	void detec_pp(Mat &src)
	{
		Mat ROI;
		Rect rect(160, 0, 320, 480);
		src2 = src.clone();
		//pyrDown(src, src1,Size(src.cols/2,src.rows/2) );
		//src(rect).copyTo(ROI);
	    //HOGDescriptor hog(Size(48, 96), Size(16, 16), Size(8, 8), Size(8, 8), 9, 1, -1, 0, 0.2, true, 64);
		//hog.setSVMDetector(HOGDescriptor::getDaimlerPeopleDetector());
		HOGDescriptor hog;
		hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());
		vector<Rect>found, found_filter;
		hog.detectMultiScale(src2, found, 0, Size(8, 8), Size(32, 32), 1.05, 2);
		if (found.size()>0)
		{
			cout << "number of detected rectangles" << found.size() << endl;

			for (int i = 0; i < found.size(); i++)
			{
				Rect r = found[i];
				int j = 0;
				for (; j < found.size(); j++)
				{
					if (j != i && (r&found[j]) == r)
					{
						break;
					}
				}
				if (j == found.size())
				{
					found_filter.push_back(r);
				}
			}

			cout << "number of people detected" << found_filter.size() << endl;
			for (int i = 0; i < found_filter.size(); i++)
			{
				Rect r = found_filter[i];
				r.x += cvRound(r.width*0.1);
				r.y += cvRound(r.height*0.07);
				r.width = cvRound(r.width*0.8);
				r.height = cvRound(r.height*0.8);
				rectangle(src2, r.tl(), r.br(), Scalar(255, 0, 0), 2, 8);
			}
		}		
	}
};