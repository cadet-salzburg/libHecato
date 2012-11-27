#include "HTKalmanFilter.h"

using namespace cv;

HTKalmanFilter::HTKalmanFilter(float initX, float initY) : kalmanFilter(4, 2, 0), state(4, 1), processNoise(4, 1, CV_32F), measurement(2,1)
{
	measurement.setTo(Scalar(0));

	kalmanFilter.statePre.at<float>(0, 0) = kalmanFilter.statePost.at<float>(0, 0) = measurement.at<float>(0, 0) = initX;
	kalmanFilter.statePre.at<float>(0, 1) = kalmanFilter.statePost.at<float>(0, 1) = measurement.at<float>(0, 1) = initY;
	kalmanFilter.statePre.at<float>(0, 2) = kalmanFilter.statePost.at<float>(0, 2) = 0;
	kalmanFilter.statePre.at<float>(0, 3) = kalmanFilter.statePost.at<float>(0, 3) = 0;
	//x   y  dx  dy
	kalmanFilter.transitionMatrix = *(Mat_<float>(4, 4) << 1,  0,  1,  0,
		0,  1,  0,  1,
		0,  0,  1,  0,
		0,  0,  0,  1);

	setIdentity(kalmanFilter.measurementMatrix);
	setIdentity(kalmanFilter.processNoiseCov, Scalar::all(1e-4));
	setIdentity(kalmanFilter.measurementNoiseCov, Scalar::all(1e-1));
	setIdentity(kalmanFilter.errorCovPost, Scalar::all(.1));
}

HTKalmanFilter::~HTKalmanFilter()
{

}

void HTKalmanFilter::updateMeasurement(float posX, float posY, float* returnX, float* returnY)
{
	kalmanFilter.predict();

	measurement(0,0) = posX;
	measurement(0,1) = posY;
	//NEW
	// measurement += kalmanFilter.measurementMatrix*state;
	Mat estimation = kalmanFilter.correct(measurement);
	curX = estimation.at<float>(0, 0);
	curY = estimation.at<float>(0, 1);
	if (returnX)
		*returnX = curX;
	if (returnY)
		*returnY = curY;

	state = kalmanFilter.transitionMatrix*state;
}

float HTKalmanFilter::getX()
{
	return curX;
}

float HTKalmanFilter::getY()
{
	return curY;
}
