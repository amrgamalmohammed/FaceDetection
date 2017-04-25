#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include "Tserial.h"

using namespace std;
using namespace cv;

void handleFrames (Mat frame);

String face_cascade_name = "E:\\CS\\OpenCV\\opencv\\build\\etc\\haarcascades\\haarcascade_frontalface_alt.xml";
String eyes_cascade_name = "E:\\CS\\OpenCV\\opencv\\build\\etc\\haarcascades\\haarcascade_eye.xml";
CascadeClassifier face_cascade;
CascadeClassifier eyes_cascade;

// Serial to Arduino
int arduino_command;
Tserial *arduino_com;
short MSBLSB = 0;
unsigned char MSB = 0;
unsigned char LSB = 0;
int cameraPort = 1;
char *comPort = "COM6";
int baudrate = 57600;


int main(int argc, const char** argv)
{
	VideoCapture capture;
	Mat frame;

	// serial to Arduino setup 
	arduino_com = new Tserial();
	if (arduino_com != 0) {
		arduino_com->connect(comPort, baudrate, spNONE);
	}

	//Load the cascades
	if (!face_cascade.load(face_cascade_name)) { 
		printf("Error loading\n");
		return -1; 
	}

	if (!eyes_cascade.load(eyes_cascade_name)) { 
		printf("Error loading\n"); 
		return -1; 
	};

	//Open usb camera port
	capture.open(cameraPort);

	//Read the video stream
	if (capture.isOpened())
	{
		while (true)
		{
			capture >> frame;

			if (!frame.empty())
			{
				handleFrames(frame);
			}
			else
			{
				printf("No captured frame\n"); 
				break;
			}

			int c = waitKey(10);
			if ((char)c == 'c') {
				break; 
			}
		}
	}

	// Serial to Arduino shutdown
	arduino_com->disconnect();
	delete arduino_com;
	arduino_com = 0;

	return 0;
}


void handleFrames(Mat frame)
{

	std::vector<Rect> faces;
	Mat frame_gray;

	cvtColor(frame, frame_gray, CV_BGR2GRAY);
	equalizeHist(frame_gray, frame_gray);

	//Detect faces
	face_cascade.detectMultiScale(frame_gray, faces, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(30, 30));

	for (int i = 0; i < faces.size(); i++)
	{
		Point center(faces[i].x + faces[i].width*0.5, faces[i].y + faces[i].height*0.5);
		ellipse(frame, center, Size(faces[i].width*0.5, faces[i].height*0.5), 0, 0, 360, Scalar(255, 0, 255), 2, 8, 0);

		//Read X position of face center	
		LSB = faces[i].x & 0xff;
		MSB = (faces[i].x >> 8) & 0xff;
		// send X position of face center to serial com port
		arduino_com->sendChar(MSB);
		arduino_com->sendChar(LSB);

		//Read Y position of face center
		LSB = faces[i].y & 0xff;
		MSB = (faces[i].y >> 8) & 0xff;
		// send Y position of face center to serial com port
		arduino_com->sendChar(MSB);
		arduino_com->sendChar(LSB);

		//Mat faceROI = frame_gray(faces[i]);
	}

	//Show frames on screen
	imshow("Face Detector", frame);

}
