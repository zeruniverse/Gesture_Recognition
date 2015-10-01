#include "stdafx.h"
#include"cvheader.h"
using namespace cv;
using namespace std;
Mat myframe;
int ismyframeuse = 0;
int palm_radius;
extern cv::CascadeClassifier face_cascade;
extern IplImage* mybackground;
float radius_palm_center;
CvMemStorage* palm_st;
CvMemStorage* st;
CvMemStorage* dft_st;
CvMemStorage* hull_st;
CvMemStorage* finger_st;
CvMemStorage* fingerdft_st;
CvSeq* contours;
CvSeq* hull;
CvSeq* defect;
CvSeq* palm;
CvSeq* finger_dft;
CvSeq* fingerseq;
CvPoint armcenter, p, palm_center;
CvBox2D contour_center;
vector<Rect> faces;

Rect facedetect(Mat frame, CascadeClassifier facecad)
{
	Mat frame_gray;
	
	Rect p;
	//Initialize
	p.height = 0;
	p.width = 0;
	p.x = 0;
	int maxarea = -1;
	int maxareai = -1;
	p.y = 0;
	cv::cvtColor(frame, frame_gray, CV_BGR2GRAY);
	cv::equalizeHist(frame_gray, frame_gray);
	facecad.detectMultiScale(frame_gray, faces, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, cv::Size(30, 30));
	for (int i = 0; i < faces.size();i++) //consider the first face
	{
		cv::Point center(faces[i].x + faces[i].width*0.5, faces[i].y + faces[i].height*0.5);
		maxareai = (faces[i].area()>maxarea) ? i : maxareai;
		maxarea = (faces[i].area()>maxarea) ? faces[i].area() : maxarea;
		ellipse(myframe, center, cv::Size(faces[i].width*0.5, faces[i].height*0.5), 0, 0, 360, cv::Scalar(255, 0, 255), 4, 8, 0);
	}
	if(faces.size()!=0) p = faces[maxareai];
	return p;
}

void SkinColorModel(Mat frame, Rect faceregion, int* ymax, int* ymin, int* crmax, int* crmin, int* cbmax, int* cbmin)
{
	int y, cb, cr,r,b,g,gray;
	Mat p;
	cv::cvtColor(frame, p, CV_BGR2YCrCb);
	*crmax = -1;
	*crmin = 295;
	*cbmax = -1;
	*cbmin = 295;
	*ymax = 295;
	*ymin = -1;
	if (faceregion.area() > 5)
	{

		for (int i = faceregion.x; i < faceregion.x + faceregion.width&& i < frame.cols; i++)
		{
			for (int j = faceregion.y; j < faceregion.y + faceregion.height&& j<frame.rows; j++)
			{

				b = frame.at<cv::Vec3b>(j, i)[0];
				g = frame.at<cv::Vec3b>(j, i)[1];
				r = frame.at<cv::Vec3b>(j, i)[2];
				y = p.at<cv::Vec3b>(j, i)[0];
				cr = p.at<cv::Vec3b>(j, i)[1];
				cb = p.at<cv::Vec3b>(j, i)[2];
				gray = (int)(0.2989 * r + 0.5870 * g + 0.1140 * b);
				if (gray<200 && gray>40 && r>g && r>b)
				{
					*ymax = (y > *ymax) ? y : *ymax;
					*ymin = (y < *ymin) ? y : *ymin;
					*crmax = (cr > *crmax) ? cr : *crmax;
					*crmin = (cr < *crmin) ? cr : *crmin;
					*cbmax = (cb > *cbmax) ? cb : *cbmax;
					*cbmin = (cb < *cbmin) ? cb : *cbmin;
				}
			}
		}
		/**ymin = *ymin - 10;
		*ymax = *ymax + 10;
		*crmin = *crmin - 10;
		*crmax = *crmax + 10;
		*cbmin = *cbmin - 10;
		*cbmax = *cbmax + 10;*/
	}
	else
	{
		*ymax = 255;//(*ymax>163) ? 163 : *ymax;
		*ymin = 0;// (*ymin < 54) ? 54 : *ymin;
		*crmax = 173;// (*crmax > 173) ? 173 : *crmax;
		*crmin = 133;// (*crmin < 133) ? 133 : *crmin;
		*cbmax = 127;// (*cbmax > 127) ? 127 : *cbmax;
		*cbmin = 77;// (*cbmin < 77) ? 77 : *cbmin;
	}
	/**crmax = (*crmax > 173) ? 173 : *crmax;
	*crmin = (*crmin < 133) ? 133 : *crmin;
	*cbmax = (*cbmax > 127) ? 127 : *cbmax;
	*cbmin = (*cbmin < 77) ? 77 : *cbmin;*/
}

void Get_hull()
{
	IplImage frame = myframe;
	hull = cvConvexHull2(contours, hull_st, CV_CLOCKWISE, 0);
	CvPoint pt_tmp = **CV_GET_SEQ_ELEM(CvPoint*, hull, hull->total - 1);
	CvPoint pt;
	cvClearSeq(fingerseq);
	for (int i = 0; i < hull->total; i++)
	{
		pt = **CV_GET_SEQ_ELEM(CvPoint*, hull, i);
		//printf("%d,%d\n",pt.x,pt.y);
		cvLine(&frame, pt_tmp, pt, CV_RGB(128, 128, 128), 2, 8, 0);
		pt_tmp = pt;
		cvSeqPush(fingerseq, &pt);
		//cvCircle(&frame, pt, 5, CV_RGB(255,255,255), -1, CV_AA, 0);
	}

	defect = cvConvexityDefects(contours, hull, dft_st); 
	cvClearSeq(palm);
	for (int i = 0; i<defect->total; i++)
	{
		CvConvexityDefect* d = (CvConvexityDefect*)cvGetSeqElem(defect, i);
		if (d->depth > 10)
		{
			p.x = d->depth_point->x;
			p.y = d->depth_point->y;
			cvCircle(&frame, p, 5, CV_RGB(255, 0, 0), -1, CV_AA, 0);
			cvSeqPush(palm, &p);
		}

	}

}

int Get_Palm_Center()
{
	CvPoint distemp;
	int lengthtemp;
	int mydft = 0;
	palm_center.x = armcenter.x;
	palm_center.y = armcenter.y;
	IplImage frame = myframe;
	if (palm->total > 0)
	{
		
		palm_center.x = 0;
		palm_center.y = 0;
		for (int i = 0; i < palm->total; i++)
		{
			CvPoint *temp = (CvPoint*)cvGetSeqElem(palm, i);
			palm_center.x += temp->x;
			palm_center.y += temp->y;
		}

		palm_center.x = (int)(palm_center.x / palm->total);
		palm_center.y = (int)(palm_center.y / palm->total);
		palm_radius = 0;
		
		for (int i = 0; i < palm->total; i++)
		{
			CvPoint *temp = (CvPoint*)cvGetSeqElem(palm, i);
			distemp.x = temp->x - palm_center.x;
			distemp.y = temp->y - palm_center.y;
			lengthtemp = sqrtf((distemp.x* distemp.x) + (distemp.y*distemp.y));
			palm_radius += lengthtemp;
		}

		palm_radius = (int)(palm_radius / palm->total);
		if (palm_center.y > armcenter.y) {
			palm_center.x = armcenter.x;
			palm_center.y = armcenter.y;
		}
		if (palm->total < 3) palm_radius = 0;
		cvCircle(&frame, palm_center, 5, CV_RGB(0, 255, 0), -1, CV_AA, 0);
		ellipse(myframe, palm_center, cv::Size(palm_radius, palm_radius), 0, 0, 360, cv::Scalar(255, 0, 0), 4, 8, 0);
		cvClearSeq(finger_dft);
		
		for (int i = 0; i < palm->total; i++)
		{
			CvPoint *temp = (CvPoint*)cvGetSeqElem(palm, i);
			p.x = temp->x;
			p.y = temp->y;
			if (palm_center.x - palm_radius*1.9<p.x && palm_center.x + palm_radius*1.9>p.x&&palm_center.y - palm_radius*1.5<p.y&&palm_center.y + palm_radius*0.8>p.y)
			{
				mydft++;
				cvSeqPush(finger_dft, &p);
				cvCircle(&frame, p, 5, CV_RGB(0, 255, 255), -1, CV_AA, 0);
			}
		}
		
	}
	
	return mydft;
}

IplImage* HandDetection(Mat frame, Rect faceregion, int ymax, int ymin, int crmax, int crmin, int cbmax, int cbmin)
{
	IplImage image = frame;
	IplImage myframe_ipl = myframe;
	CvScalar  fillblack = cvScalar(0);
	CvSize sz = cvGetSize(&image);
	CvSeq* maxrecord=NULL;
	int max_contour_size=-1;
	IplImage* mask = cvCreateImage(sz, 8, 1);
	Mat maskmat(mask);
	if (faceregion.area() > 5)
	{
		if (faceregion.y > faceregion.height / 4)
		{
			faceregion.y -= faceregion.height / 4;
			faceregion.height += faceregion.height / 4;
		}
		else
		{
			faceregion.height += faceregion.y;
			faceregion.y = 0;

		}
		//avoid noise for T-shirt
		faceregion.height += faceregion.height / 2;
	}
	
	int y,cr,cb;
	//Turn to YCrCb
	Mat p,b;
	cv::cvtColor(frame, p, CV_BGR2YCrCb);

	for (int i = 0; i < frame.cols;i++)
	for (int j = 0; j < frame.rows; j++)
	{

		y = p.at<cv::Vec3b>(j, i)[0];
		cr = p.at<cv::Vec3b>(j, i)[1];
		cb = p.at<cv::Vec3b>(j, i)[2];
		if (y>ymin && y<ymax && cr<crmax && cr>crmin && cb<cbmax && cb>cbmin)
			maskmat.at<unsigned char>(j,i)= 255;
		else maskmat.at<unsigned char>(j, i) = 0;
		
		if (mybackground != NULL)
		{
			b = mybackground;
			if (abs((int)frame.at<cv::Vec3b>(j, i)[0] - (int)b.at<cv::Vec3b>(j, i)[0])<10 && abs((int)frame.at<cv::Vec3b>(j, i)[1] - (int)b.at<cv::Vec3b>(j, i)[1])<10 && abs((int)frame.at<cv::Vec3b>(j, i)[2] - (int)b.at<cv::Vec3b>(j, i)[2])<10)
				maskmat.at<unsigned char>(j, i) = 0;
		}

	}
	for (int i = 0; i < faces.size(); i++)
	for (int j = faces[i].x; j < faces[i].x + faces[i].width; j++)
	for (int k = faces[i].y; k < faces[i].y + faces[i].height; k++)
		maskmat.at<unsigned char>(k,j) = 0;

	/*for (int i = faceregion.x; i < faceregion.x + faceregion.width; i++)
	{
		for (int j = faceregion.y; j < faceregion.y + faceregion.height; j++)
		{
			cvSet2D(mask, j,i, fillblack);
		}
	}*/
	cvErode(mask, mask, 0, 1); //ERODE first then DILATE to eliminate the noises.
	cvErode(mask, mask, 0, 1); //ERODE first then DILATE to eliminate the noises.
	cvDilate(mask, mask, 0, 1);
	cvFindContours(mask, st, &contours, sizeof(CvContour),CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, cvPoint(0, 0));
	while (contours)
	{
		if (contours->total >= 370 && contours->total > max_contour_size)
		{
			max_contour_size = contours->total;
			maxrecord = contours;
		}
		contours = contours->h_next;
	}
	//maxrecord->h_next = NULL;
	//maxrecord->h_prev = NULL;
	//maxrecord->header_size = 1;
	contours = maxrecord;
	cvDrawContours(mask, maxrecord, cvScalar(100), cvScalar(50), 1, 2, CV_AA, cvPoint(0, 0));
	if (maxrecord)
	{
		contour_center = cvMinAreaRect2(maxrecord, 0);
		armcenter.x = cvRound(contour_center.center.x);
		armcenter.y = cvRound(contour_center.center.y);
		cvCircle(&myframe_ipl,armcenter,10,CV_RGB(255,255,255),-1,8,0);
		//find palm center
		/*double dist,maxdist=-1;
		Point center;
		vector<Point2f> cont_seq;
		for (int i = 0; i < max_contour_size; i++){
			int* point = (int*)cvGetSeqElem(maxrecord, i);
			cont_seq.insert(cont_seq.end(), Point2f(point[0], point[1]));
		}
		for (int i = 0; i< frame.cols; i++)
		{
			for (int j = 0; j< frame.rows; j++)
			{
				
				dist = pointPolygonTest(cont_seq, cv::Point2f(i, j), true);
				if (dist > maxdist)
				{
					maxdist = dist;
					center = cv::Point(i, j);
				}
			}
		}
		cvCircle(&myframe_ipl, center, 10, CV_RGB(255, 0,0), -1, 8, 0);*/
		Get_hull();
		
		
	}

	return mask;
}
int qcompare(const void * a, const void * b)
{
	return (*(int*)a - *(int*)b);
}
struct mypoint{
	int x;
	int y;
};
int qcompare1(const void * a, const void * b)
{
	struct mypoint *ta = (struct mypoint *)a;
	struct mypoint *tb = (struct mypoint *)b;
	return (ta->x - tb->x);
}
double get_cos_value(CvPoint b, struct mypoint c)//palm_center - base point
{
	CvPoint a;
	a.x = palm_center.x;
	a.y = palm_center.y;
	int vec1x = b.x - a.x;
	int vec1y = b.y - a.y;
	int vec2x = c.x - a.x;
	int vec2y = c.y - a.y;
	return ((double)(vec1x*vec2x + vec1y*vec2y)) / (sqrt((double)((vec1x*vec1x) + (vec1y*vec1y)))*sqrt((double)((vec2x*vec2x) + (vec2y*vec2y))));
}
int Get_fingertip() //number of fingertips
{
	struct mypoint gaps[150];
	struct mypoint possible_tips[150];
	vector<struct mypoint> checked_tips;
	struct mypoint mypoint_temp;
	IplImage frame = myframe;
	CvPoint tmp_cvpnt;
	int cnt_finger = 0;
	int pnt = 0;
	if (palm_radius == 0) return 0;
	for (int i = 0; i < finger_dft->total; i++)
	{
		CvPoint *temp = (CvPoint*)cvGetSeqElem(finger_dft, i);
		gaps[i].x = temp->x;
		gaps[i].y = temp->y;
	}
	gaps[finger_dft->total].x = -1;
	gaps[finger_dft->total].y = 0;//lower bound
	gaps[finger_dft->total + 1].x = 30000;
	gaps[finger_dft->total + 1].y = 0;
	gaps[finger_dft->total+2].x = 30001;
	gaps[finger_dft->total + 2].y = 30001;//higher bound
	std::qsort(gaps, finger_dft->total + 3, sizeof(struct mypoint), qcompare1);
	for (int i = 0; i < fingerseq->total; i++)
	{
		CvPoint *temp = (CvPoint*)cvGetSeqElem(fingerseq, i);
		possible_tips[i].x = temp->x;
		possible_tips[i].y = temp->y;
	}
	std::qsort(possible_tips, fingerseq->total, sizeof(struct mypoint), qcompare1);
	mypoint_temp.x = -1;
	mypoint_temp.y = -1;
	tmp_cvpnt.x = palm_center.x;
	tmp_cvpnt.y = 999;
	for (int i = 0; i < fingerseq->total; i++)
	{
		//p.x = possible_tips[i].x;
		//p.y = possible_tips[i].y;
		//cvCircle(&frame, p, 5, CV_RGB(100, 0, 200), -1, CV_AA, 0);
		if (/*(possible_tips[i].x>gaps[pnt].x || gaps[pnt].x==30000) &&*/get_cos_value(tmp_cvpnt, possible_tips[i])<0.98 &&possible_tips[i].y<palm_center.y + 0.8*palm_radius && ((palm_center.x - possible_tips[i].x)*(palm_center.x - possible_tips[i].x)) + ((palm_center.y - possible_tips[i].y)*(palm_center.y - possible_tips[i].y))>palm_radius*palm_radius*3.5)
		{
			cnt_finger++;
			tmp_cvpnt.x = possible_tips[i].x;
			tmp_cvpnt.y = possible_tips[i].y;
			p.x = possible_tips[i].x;
			p.y = possible_tips[i].y;
			//checked_tips.push_back(mypoint_temp);
			cvCircle(&frame, p, 5, CV_RGB(0,0,0), -1, CV_AA, 0);
			//while (gaps[++pnt].x < possible_tips[i].x);
		}
	}
	
	/*for (int i = 0; i < checked_tips.size(); i++)
	{
		if (get_cos_value(mypoint_temp, checked_tips[i]) < 0.8)
		{
			mypoint_temp.x = checked_tips[i].x;
			mypoint_temp.y = checked_tips[i].y;
			p.x = mypoint_temp.x;
			p.y = mypoint_temp.y;
			cvCircle(&frame, p, 5, CV_RGB(0, 0, 0), -1, CV_AA, 0);
			cnt_finger++;
		}
	}*/
	char tmp[30];
	_itoa_s(cnt_finger, tmp, 10);
	CvFont Font1 = cvFont(3, 3);
	cvPutText(&frame, tmp, cvPoint(10, 50), &Font1, CV_RGB(255, 0, 0));
	//int i=pnt;
	return cnt_finger;
}



int mygesturedetect(Mat frame) //-1 undetected, 0-scissor, 1-rock, 2-paper
{
	myframe.release();
	myframe = frame.clone();
	palm_st = cvCreateMemStorage(0);
	st = cvCreateMemStorage(0);
	dft_st = cvCreateMemStorage(0);
	hull_st = cvCreateMemStorage(0);
	finger_st = cvCreateMemStorage(0);
	fingerdft_st = cvCreateMemStorage(0);
	palm = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint), palm_st);
	finger_dft = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint), fingerdft_st);
	fingerseq = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint), finger_st);
	Rect faceregion;
	int rmax, rmin, gmax, gmin, bmax, bmin;
	faceregion = facedetect(frame, face_cascade);
	//faceregion = handdetect_haar(frame, fist_cascade);
	SkinColorModel(frame, faceregion, &rmax, &rmin, &gmax, &gmin, &bmax, &bmin);
	IplImage* p = HandDetection(frame, faceregion, rmax, rmin, gmax, gmin, bmax, bmin);
	int dfts = Get_Palm_Center();
	//myframe = p;
	int tips=Get_fingertip();
	int flag = (contours == NULL) ? 1 : 0;
	cv::imshow("GESTURE RECOGNITION", myframe);
	cvReleaseImage(&p);
	cvReleaseMemStorage(&st);
	cvReleaseMemStorage(&fingerdft_st);
	cvReleaseMemStorage(&palm_st);
	cvReleaseMemStorage(&dft_st);
	cvReleaseMemStorage(&hull_st);
	vector<Rect>().swap(faces);
	faces.clear();
	if (flag) return -1;
	if (tips >= 4 && dfts >= 3 && tips<=6&&dfts<=5) return 2; //paper
	if (tips ==0 && dfts>=2 && dfts<=5) return 2;//paper, special case 1 (open palm with all fingers together)
	if (tips == 0) return 1;//rock
	if (tips >= 1 && tips <= 2 && dfts >= 2 && dfts <= 4) return 0;//scissors
	if (tips == 3 && dfts >= 2 && dfts <= 3) return 0;//scissors
	if (tips == 3 && dfts >= 4 && dfts <= 5) return 2;//paper
	return -1;
}