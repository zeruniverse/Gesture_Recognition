#include "CvvImage.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <vector>
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

int mygesturedetect(cv::Mat frame); //-1 undetected, 0-scissor, 1-rock, 2-paper