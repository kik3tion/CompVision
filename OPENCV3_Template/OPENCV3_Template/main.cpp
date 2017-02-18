#include "opencv2\opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;
using namespace cv;

const float calibrationSquareDImension = 0.024f; //meters
const float arucoSquareDimension = 0.1016f; //meters
const Size chessboardDimensions = Size(6,9);

void createKnownBoardPosition(Size boardSize, float squareEdgeLength, vector<Point3f>& corners) {
	for (int i = 0; i < boardSize.height; i++) {
		for (int j = 0; j < boardSize.width; j++) {
			corners.push_back(Point3f(j*squareEdgeLength, i *squareEdgeLength, 0.0f));
		}
	}
}

void getChessboardCorners(vector<Mat> images, vector<vector<Point2f>>& allFoundCorners, bool showResults = false) {
	for (vector<Mat>::iterator iter = images.begin(); iter != images.end(); iter++) {
		vector<Point2f> pointBuf;
		bool found = findChessboardCorners(*iter, Size(9,6), pointBuf, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_NORMALIZE_IMAGE);

		if (found) {
			allFoundCorners.push_back(pointBuf);
		}
		if (showResults) {
			drawChessboardCorners(*iter, Size(9,6), pointBuf, found);
			imshow("Looking for Corners",*iter);
			waitKey(0);
		}
	}
}

void cameraCalibration(vector<Mat> calibrationImages, Size boardSize, float squareEdgeLength, Mat& cameraMatrix, Mat& distanceCoefficients)
{
	vector<vector<Point2f>> checkerboardImagesSpacePoints;
	getChessboardCorners(calibrationImages,checkerboardImagesSpacePoints,false);
	vector<vector<Point3f>> worldSpaceCornerPoints(1);
	createKnownBoardPosition(boardSize, squareEdgeLength, worldSpaceCornerPoints[0]);

	vector<Mat> rVectors, tVectors;
	distanceCoefficients= Mat::zeros(8, 1, CV_64F);

	calibrateCamera(worldSpaceCornerPoints, checkerboardImagesSpacePoints, boardSize, cameraMatrix, distanceCoefficients, rVectors, tVectors);


}

bool saveCameraCalibration(string name, Mat cameraMatrix, Mat distanceCoefficients) 
{
	ofstream outStream(name);
	if(outStream)
	{
		uint16_t rows = cameraMatrix.rows;
		uint16_t columns = cameraMatrix.cols;

		for (int r = 0; r < rows; r++)
		{ 
			for (int c = 0; c < columns; c++) 
			{ 
				double value = cameraMatrix.at<double>(r, c); 
				outStream << value << endl; 
			}
		}

		rows = distanceCoefficients.rows;
		columns = distanceCoefficients.cols;
		
		for (int r = 0; r < rows; r++)
		{
			for (int c = 0; c < columns; c++)
			{
				double value = distanceCoefficients.at<double>(r, c);
				outStream << value << endl;
			}
		}

		outStream.close();
		return true;
	}
	return false;
}

int main(int argv, char** argc) {
	/*Mat test = imread("WIN_20170215_161600.JPG", CV_LOAD_IMAGE_UNCHANGED);
	imshow("test",test);
	waitKey();*/

	Mat frame;
	Mat drawToFrame;

	Mat cameraMatrix = Mat::eye(3,3,CV_64F);
	Mat distanceCoefficients;

	vector<Mat> savedImage;

	vector<vector<Point2f>> markerCorners, rejectedCandidates;

	VideoCapture vid(0);

	if (!vid.isOpened()) {
		//if something goes wrong here it would stop
		return 0;
		
	}
	int framesPerSecond = 20;
	namedWindow("Webcam", CV_WINDOW_AUTOSIZE);

	while (true) {
		if (!vid.read(frame))
			break;
		vector<Vec2f> foundPoints;
		bool found = false;

		found = findChessboardCorners(frame, chessboardDimensions, foundPoints, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_NORMALIZE_IMAGE);
		frame.copyTo(drawToFrame);
		drawChessboardCorners(drawToFrame, chessboardDimensions, foundPoints, found);
		if (found){
			/*line(drawToFrame, foundPoints.front, (Point)foundPoints.at(6), Scalar(255, 0, 0), 1, 8, 0);
			line(drawToFrame, foundPoints.front, (Point)foundPoints.at(1), Scalar(0,255, 0), 1, 8, 0);
			line(drawToFrame, foundPoints.front, (Point)foundPoints.at(6), Scalar(0, 0, 255), 1, 8, 0);
			
			cvDotProduct(foundPoints.front,foundPoints.at(6));*/
			imshow("Webcam", drawToFrame);
		}
		else
			imshow("Webcam", frame);
		char character = waitKey(1000 / framesPerSecond);

		switch(character)
		{
		case ' ':
			//saving image
			if(found)
			{
				Mat temp;
				frame.copyTo(temp);
				savedImage.push_back(temp);
				
			}
			break;
		case 13:
			//start calibration
			if(savedImage.size()>5)
			{
				cameraCalibration(savedImage, chessboardDimensions, calibrationSquareDImension, cameraMatrix, distanceCoefficients);
				saveCameraCalibration("CameraCalibration for Aaron", cameraMatrix,distanceCoefficients);
			}
			break;
		case 27:
			//exit
			return 0;
			break;
		}
	}

	return 0;
}