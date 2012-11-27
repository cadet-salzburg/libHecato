#include <opencv.hpp>

class HTKalmanFilter
{
public:
	HTKalmanFilter(float initX, float initY);
	~HTKalmanFilter();

	void updateMeasurement(float posX, float posY, float* returnX = 0, float* returnY = 0);
	float getX();
	float getY();


private:
	HTKalmanFilter();
	HTKalmanFilter(const HTKalmanFilter&);
	cv::KalmanFilter kalmanFilter;
	cv::Mat_<float> state; /* (x, y, Vx, Vy) */
	cv::Mat processNoise;
	cv::Mat_<float> measurement;
	float curX, curY;
};
