#include<opencv2/objdetect/objdetect.hpp>
#include"lane_tracker.h"
#include"light_recog.h"
#include"people.h"
//#include"car_plate.h"
//#include"car.h"
#include"light.h"
#include"vehicle_plate.h"
#include"utils.h"


#undef MIN
#undef MAX
#define MAX(a,b) ((a)<(b)?(b):(a))
#define MIN(a,b) ((a)>(b)?(b):(a))

void crop(IplImage* src, IplImage* dest, CvRect rect) {
	cvSetImageROI(src, rect);
	cvCopy(src, dest);
	cvResetImageROI(src);
}

struct Lane {
	Lane(){}
	Lane(CvPoint a, CvPoint b, float angle, float kl, float bl) : p0(a), p1(b), angle(angle),
		votes(0), visited(false), found(false), k(kl), b(bl) { }

	CvPoint p0, p1;
	int votes;
	bool visited, found;
	float angle, k, b;
};

struct Status {
	Status() :reset(true), lost(0){}
	ExpMovingAverage k, b;
	bool reset;
	int lost;
};

struct Vehicle {
	CvPoint bmin, bmax;
	int symmetryX;
	bool valid;
	unsigned int lastUpdate;
};

struct VehicleSample {
	CvPoint center;
	float radi;
	unsigned int frameDetected;
	int vehicleIndex;
};

#define GREEN CV_RGB(0,255,0)
#define RED CV_RGB(255,0,0)
#define BLUE CV_RGB(255,0,255)
#define PURPLE CV_RGB(255,0,255)

Status laneR, laneL;
std::vector<Vehicle> vehicles;
std::vector<VehicleSample> samples;

enum{
	SCAN_STEP = 5,			  // in pixels
	LINE_REJECT_DEGREES = 10, // in degrees
	BW_TRESHOLD = 250,		  // edge response strength to recognize for 'WHITE'
	BORDERX = 10,			  // px, skip this much from left & right borders
	MAX_RESPONSE_DIST = 5,	  // px

	//CANNY_MIN_TRESHOLD = 1;
	CANNY_MIN_TRESHOLD = 50,	  // edge detector minimum hysteresis threshold
	CANNY_MAX_TRESHOLD = 100, // edge detector maximum hysteresis threshold

	HOUGH_TRESHOLD = 50,		// line approval vote threshold
	HOUGH_MIN_LINE_LENGTH = 50,	// remove lines shorter than this treshold
	HOUGH_MAX_LINE_GAP = 100,   // join lines to one with smaller than this gaps

	CAR_DETECT_LINES = 6,    // minimum lines for a region to pass validation as a 'CAR'
	CAR_H_LINE_LENGTH = 10,  // minimum horizontal line length from car body in px

	MAX_VEHICLE_SAMPLES = 20,      // max vehicle detection sampling history
	CAR_DETECT_POSITIVE_SAMPLES = MAX_VEHICLE_SAMPLES - 2, // probability positive matches for valid car
	MAX_VEHICLE_NO_UPDATE_FREQ = 10 // remove car after this much no update frames
};

#define K_VARY_FACTOR 0.2f
#define B_VARY_FACTOR 20
#define MAX_LOST_FRAMES 30


unsigned char pixel(IplImage* img, int x, int y) {
	return (unsigned char)img->imageData[(y*img->width + x)*img->nChannels];
}

int findSymmetryAxisX(IplImage* half_frame, CvPoint bmin, CvPoint bmax) {

	float value = 0;
	int axisX = -1; // not found

	int xmin = bmin.x;
	int ymin = bmin.y;
	int xmax = bmax.x;
	int ymax = bmax.y;
	int half_width = half_frame->width / 2;
	int maxi = 1;

	for (int x = xmin, j = 0; x<xmax; x++, j++) {
		float HS = 0;
		for (int y = ymin; y<ymax; y++) {
			int row = y*half_frame->width*half_frame->nChannels;
			for (int step = 1; step<half_width; step++) {
				int neg = x - step;
				int pos = x + step;
				unsigned char Gneg = (neg < xmin) ? 0 : (unsigned char)half_frame->imageData[row + neg*half_frame->nChannels];
				unsigned char Gpos = (pos >= xmax) ? 0 : (unsigned char)half_frame->imageData[row + pos*half_frame->nChannels];
				HS += abs(Gneg - Gpos);
			}
		}

		if (axisX == -1 || value > HS) { // find minimum
			axisX = x;
			value = HS;
		}
	}

	return axisX;
}

bool hasVertResponse(IplImage* edges, int x, int y, int ymin, int ymax) {
	bool has = (pixel(edges, x, y) > BW_TRESHOLD);
	if (y - 1 >= ymin) has &= (pixel(edges, x, y - 1) < BW_TRESHOLD);
	if (y + 1 < ymax) has &= (pixel(edges, x, y + 1) < BW_TRESHOLD);
	return has;
}

int horizLine(IplImage* edges, int x, int y, CvPoint bmin, CvPoint bmax, int maxHorzGap) {

	// scan to right
	int right = 0;
	int gap = maxHorzGap;
	for (int xx = x; xx<bmax.x; xx++) {
		if (hasVertResponse(edges, xx, y, bmin.y, bmax.y)) {
			right++;
			gap = maxHorzGap; // reset
		}
		else {
			gap--;
			if (gap <= 0) {
				break;
			}
		}
	}

	int left = 0;
	gap = maxHorzGap;
	for (int xx = x - 1; xx >= bmin.x; xx--) {
		if (hasVertResponse(edges, xx, y, bmin.y, bmax.y)) {
			left++;
			gap = maxHorzGap; // reset
		}
		else {
			gap--;
			if (gap <= 0) {
				break;
			}
		}
	}

	return left + right;
}

bool vehicleValid(IplImage* half_frame, IplImage* edges, Vehicle* v, int& index) {

	index = -1;

	// first step: find horizontal symmetry axis
	v->symmetryX = findSymmetryAxisX(half_frame, v->bmin, v->bmax);
	if (v->symmetryX == -1) return false;

	// second step: cars tend to have a lot of horizontal lines
	int hlines = 0;
	for (int y = v->bmin.y; y < v->bmax.y; y++) {
		if (horizLine(edges, v->symmetryX, y, v->bmin, v->bmax, 2) > CAR_H_LINE_LENGTH) {
#if _DEBUG
			cvCircle(half_frame, cvPoint(v->symmetryX, y), 2, PURPLE);
#endif
			hlines++;
		}
	}

	int midy = (v->bmax.y + v->bmin.y) / 2;

	// third step: check with previous detected samples if car already exists
	int numClose = 0;
	float closestDist = 0;
	for (int i = 0; i < samples.size(); i++) {
		int dx = samples[i].center.x - v->symmetryX;
		int dy = samples[i].center.y - midy;
		float Rsqr = dx*dx + dy*dy;

		if (Rsqr <= samples[i].radi*samples[i].radi) {
			numClose++;
			if (index == -1 || Rsqr < closestDist) {
				index = samples[i].vehicleIndex;
				closestDist = Rsqr;
			}
		}
	}

	return (hlines >= CAR_DETECT_LINES || numClose >= CAR_DETECT_POSITIVE_SAMPLES);
}

void removeOldVehicleSamples(unsigned int currentFrame) {
	// statistical sampling - clear very old samples
	std::vector<VehicleSample> sampl;
	for (int i = 0; i < samples.size(); i++) {
		if (currentFrame - samples[i].frameDetected < MAX_VEHICLE_SAMPLES) {
			sampl.push_back(samples[i]);
		}
	}
	samples = sampl;
}

void removeSamplesByIndex(int index) {
	// statistical sampling - clear very old samples
	std::vector<VehicleSample> sampl;
	for (int i = 0; i < samples.size(); i++) {
		if (samples[i].vehicleIndex != index) {
			sampl.push_back(samples[i]);
		}
	}
	samples = sampl;
}

void removeLostVehicles(unsigned int currentFrame) {
	// remove old unknown/false vehicles & their samples, if any
	for (int i = 0; i<vehicles.size(); i++) {
		if (vehicles[i].valid && currentFrame - vehicles[i].lastUpdate >= MAX_VEHICLE_NO_UPDATE_FREQ) {
			printf("\tremoving inactive car, index = %d\n", i);
			removeSamplesByIndex(i);
			vehicles[i].valid = false;
		}
	}
}

void vehicleDetection(IplImage* half_frame, CvHaarClassifierCascade* cascade, CvMemStorage* haarStorage) {

	static unsigned int frame = 0;
	frame++;
	printf("*** vehicle detector frame: %d ***\n", frame);

	removeOldVehicleSamples(frame);

	// Haar Car detection
	const double scale_factor = 1.05; // every iteration increases scan window by 5%
	const int min_neighbours = 2; // minus 1, number of rectangles, that the object consists of
	CvSeq* rects = cvHaarDetectObjects(half_frame, cascade, haarStorage, scale_factor, min_neighbours, CV_HAAR_DO_CANNY_PRUNING);

	// Canny edge detection of the minimized frame
	if (rects->total > 0) {
		printf("\thaar detected %d car hypotheses\n", rects->total);
		IplImage *edges = cvCreateImage(cvSize(half_frame->width, half_frame->height), IPL_DEPTH_8U, 1);
		cvCanny(half_frame, edges, CANNY_MIN_TRESHOLD, CANNY_MAX_TRESHOLD);

		/* validate vehicles */
		for (int i = 0; i < rects->total; i++) {
			CvRect* rc = (CvRect*)cvGetSeqElem(rects, i);

			Vehicle v;
			v.bmin = cvPoint(rc->x, rc->y);
			v.bmax = cvPoint(rc->x + rc->width, rc->y + rc->height);
			v.valid = true;

			int index;
			if (vehicleValid(half_frame, edges, &v, index)) { // put a sample on that position

				if (index == -1) { // new car detected

					v.lastUpdate = frame;

					// re-use already created but inactive vehicles
					for (int j = 0; j<vehicles.size(); j++) {
						if (vehicles[j].valid == false) {
							index = j;
							break;
						}
					}
					if (index == -1) { // all space used
						index = vehicles.size();
						vehicles.push_back(v);
					}
					printf("\tnew car detected, index = %d\n", index);
				}
				else {
					// update the position from new data
					vehicles[index] = v;
					vehicles[index].lastUpdate = frame;
					printf("\tcar updated, index = %d\n", index);
				}

				VehicleSample vs;
				vs.frameDetected = frame;
				vs.vehicleIndex = index;
				vs.radi = (MAX(rc->width, rc->height)) / 4; // radius twice smaller - prevent false positives
				vs.center = cvPoint((v.bmin.x + v.bmax.x) / 2, (v.bmin.y + v.bmax.y) / 2);
				samples.push_back(vs);
			}
		}

		//cvShowImage("Half-frame[edges]", edges);
		//cvMoveWindow("Half-frame[edges]", half_frame->width * 2 + 10, half_frame->height);
		//cvReleaseImage(&edges);
	}
	else {
		printf("\tno vehicles detected in current frame!\n");
	}

	removeLostVehicles(frame);

	printf("\ttotal vehicles on screen: %d\n", vehicles.size());
}

void drawVehicles(IplImage* half_frame) {

	// show vehicles
	for (int i = 0; i < vehicles.size(); i++) {
		Vehicle* v = &vehicles[i];
		if (v->valid && (v->bmax.x - v->bmin.x) >= 60) {
			cvRectangle(half_frame, v->bmin, v->bmax, GREEN, 1);

			int midY = (v->bmin.y + v->bmax.y) / 2;
			cvLine(half_frame, cvPoint(v->symmetryX, midY - 10), cvPoint(v->symmetryX, midY + 10), PURPLE);
		}
	}

	// show vehicle position sampling
	/*for (int i = 0; i < samples.size(); i++) {
	cvCircle(half_frame, cvPoint(samples[i].center.x, samples[i].center.y), samples[i].radi, RED);
	}*/
}


int main()
{
	system("color 2F");
	//VideoCapture cap(0);
	VideoCapture cap("campus.mp4");

	//iplimage 读取视频
	CvCapture *input_video = cvCreateFileCapture("campus.mp4");
	//CvCapture *input_video = cvCaptureFromCAM(0);

	if (!cap.isOpened())
	{
		cout << "打开camera失败！" << endl;
		return false;
	}

	//iplimage
	CvSize video_size;
	video_size.height = (int)cvGetCaptureProperty(input_video, CV_CAP_PROP_FRAME_HEIGHT);
	video_size.width = (int)cvGetCaptureProperty(input_video, CV_CAP_PROP_FRAME_WIDTH);
	IplImage *framec = NULL;
	IplImage *half_framed = cvCreateImage(cvSize(video_size.width / 4, video_size.height / 4), IPL_DEPTH_8U, 3);
	IplImage *half_frame = cvCreateImage(cvSize(video_size.width / 4, video_size.height / 4), IPL_DEPTH_8U, 3);
	IplImage *half_framedd = cvCreateImage(cvSize(video_size.width / 2, video_size.height / 2), IPL_DEPTH_8U, 3);

	Mat frame00, frame0, frame, halfimg, framep, framel, framed, framedd;
	cap >> frame00;
	Mat M = getRotationMatrix2D(Point(frame00.cols / 2, frame00.rows / 2), 180, 1);
	warpAffine(frame00, frame0, M, frame00.size(), 1);
	Size sizeout, sizeout1, sizeout2;
	double fscale = 0.5;
	sizeout.width = frame0.cols*fscale;
	sizeout.height = frame0.rows*fscale;
	pyrDown(frame0, framed, sizeout);
	sizeout1.width = framed.cols*fscale;
	sizeout1.height = framed.rows*fscale;
	pyrDown(framed, frame, sizeout1);
	sizeout2.width = framedd.cols*fscale;
	sizeout2.height = framedd.rows*fscale;
	//pyrDown(framedd, frame, sizeout2);
	framel = frame.clone();
	framep = frame.clone();

	people pt;
	lane_tracker lt;
	//light_recog lg;
	//car cr;
	light light;
	vehicle_plate plate;

	//BackgroundSubtractorMOG2 bgsubtractor(500, 10, 0);
	CascadeClassifier vehicle;
	/*String vehicle_cascade_name = "cars3.xml";
	vehicle.load(vehicle_cascade_name);
	std::vector<Rect>cars;
	Size original_size = vehicle.getOriginalWindowSize();*/

	//iplimage vehicle_detect
	CvMemStorage* houghStorage = cvCreateMemStorage(0);
	CvMemStorage* haarStorage = cvCreateMemStorage(0);
	CvHaarClassifierCascade* cascade = (CvHaarClassifierCascade*)cvLoad("cars3.xml");
	CvMat* map = cvCreateMat(2, 3, CV_32FC1);

	while (waitKey(10) != 27)
	{

		lt.setpara(60, 20, 60, 2, 50, frame);
		halfimg = frame(Rect(0, frame.rows / 2, frame.cols, frame.rows / 2));
		lt.on_lane(halfimg);
		/*lt.on_lane(lt.minl, 0);
		lt.on_lane(lt.maxg, 0);
		lt.on_lane(lt.gsize, 0);
		lt.on_lane(lt.thre,0);*/


		/*lg.srcimg = framel;
		lg.pre_proc(lg.srcimg);
		lg.find_imgroi(lg.black);
		lg.color_recog(lg.imgroi);
		framel = lg.srcimg;*/

		//light.traffic_light(framel);

		pt.detec_pp(framep);
		framep = pt.src2;


		//iplimage detect
		framec = cvQueryFrame(input_video);

		float scale = 1;
		CvSize dec_size;
		dec_size.height = cvGetSize(framec).height*scale;
		dec_size.width = cvGetSize(framec).width*scale;
		IplImage *framecc = cvCreateImage(dec_size, framec->depth, framec->nChannels);
		cvResize(framec, framecc, 1);
		cvPyrDown(framecc, half_framedd, CV_GAUSSIAN_5x5); // Reduce the image by 8
		cvPyrDown(half_framedd, half_framed, CV_GAUSSIAN_5x5);
		cv2DRotationMatrix(cvPoint2D32f(0.5*half_framed->width, 0.5*half_framed->height), 180, 1, map);
		cvWarpAffine(half_framed, half_frame,map);
		//cvPyrDown(half_framedd, half_frame, CV_GAUSSIAN_5x5);

		// process vehicles
		vehicleDetection(half_frame, cascade, haarStorage);
		drawVehicles(half_frame);
		cvShowImage("Half-frame", half_frame);
		//cvMoveWindow("Half-frame", half_frame->width * 2 + 10, 0);


		imshow("车道线", halfimg);
		imshow("行人", framep);
		//imshow("红绿灯", framel);
		//imshow("车辆", frame);
		//imshow("透视变换", frame0);

		cap >> frame00;
		Mat M = getRotationMatrix2D(Point(frame00.cols / 2, frame00.rows / 2), 180, 1);
		warpAffine(frame00, frame0, M, frame00.size(), 1);
		pyrDown(frame0, framed, sizeout);
		pyrDown(framed, frame, sizeout1);
		//pyrDown(framedd, frame, sizeout2);
		framep = frame.clone();
		framel = frame.clone();

		if (waitKey(10) >= 0)
		{
			break;
		}

	}
	return 0;
	cvReleaseHaarClassifierCascade(&cascade);
	cvReleaseMemStorage(&haarStorage);
	cvReleaseImage(&half_frame);

	cvReleaseCapture(&input_video);
}

