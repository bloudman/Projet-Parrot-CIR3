#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>



#include <control.h>
#include <image.h>
#include <unistd.h>

using namespace cv;
using namespace std;


class ImageProcessing : public sumo::Image
{
public:
	void handleImage(const struct sumo::image *, const uint8_t *buffer, size_t size)
	{
    cout << "receive frame" << endl; //ttest
    std::vector<uint8_t> v;
    //v.assign(buffer, buffer+size);
    //cv::Mat img = cv::imdecode(v, CV_LOAD_IMAGE_COLOR);
		fprintf(stderr, "received image of %zu bytes at %p\n", size, buffer);
	}
};
int H_MIN = 0;
int H_MAX = 255;
int S_MIN = 0;
int S_MAX = 255;
int V_MIN = 0;
int V_MAX = 255;


int DetectLines(Mat& src, Mat& dst)
{
    Mat cdst;
    Canny(src, dst, 50, 200, 3);
    cvtColor(dst, cdst, COLOR_GRAY2BGR);

    vector<Vec4i> lines;
    HoughLinesP(dst, lines, 1, CV_PI / 180, 50, 50, 10);
    //if (lines.size()>300000){
    for (size_t i = 0; i < lines.size(); i++)
    {
        Vec4i l = lines[i];
        line(cdst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 3, 2);
    }
    //}

    return 0;
}

int DetectRed(Mat& src, Mat& dst){
   Mat3b bgr = src;

   Mat3b hsv;
   cvtColor(bgr, hsv, COLOR_BGR2HSV);

    Mat1b mask1, mask2;
    inRange(hsv, Scalar(H_MIN, S_MIN, V_MIN), Scalar(H_MAX, S_MAX, V_MAX), mask1);
    //inRange(hsv, Scalar(170, 70, 50), Scalar(180, 255, 255), mask2);

    //inRange(bgr, Scalar(90, 235, 235), Scalar(0, 0, 85), mask2); //BGR

    Mat1b mask = mask1 | mask2;
    dst = mask;
}

sumo::Control * Sumo;


void on_trackbar(int, void*)
{//This function gets called whenever a
	// trackbar position is changed
	cout <<"H: " <<  H_MIN << ", " << H_MAX << endl;
}
void createTrackbars()
{
	String trackbarWindowName = "TrackBars";
	namedWindow(trackbarWindowName, WINDOW_NORMAL);
	createTrackbar("H_MIN", trackbarWindowName, &H_MIN, H_MAX, on_trackbar);
	createTrackbar("H_MAX", trackbarWindowName, &H_MAX, H_MAX, on_trackbar);
	createTrackbar("S_MIN", trackbarWindowName, &S_MIN, S_MAX, on_trackbar);
	createTrackbar("S_MAX", trackbarWindowName, &S_MAX, S_MAX, on_trackbar);
	createTrackbar("V_MIN", trackbarWindowName, &V_MIN, V_MAX, on_trackbar);
	createTrackbar("V_MAX", trackbarWindowName, &V_MAX, V_MAX, on_trackbar);
}

int main(int argc, char** argv)
{

		createTrackbars();
		on_trackbar(0, 0);

		//Ouverture du sumo
	  Sumo = new sumo::Control(new ImageProcessing);
		//Sumo->open();



    VideoCapture stream1(0);
    while(true){
      Mat cameraFrame;
      Mat dest;
      stream1.read(cameraFrame);
      flip(cameraFrame, cameraFrame, 1);
      Mat cameraFrameOrigin = cameraFrame;
      DetectRed(cameraFrame, cameraFrame);
      Mat element = Mat();


  /// Apply the erosion operation
  erode( cameraFrame, cameraFrame, element);
   dilate( cameraFrame, cameraFrame, element );
    dilate( cameraFrame, cameraFrame, element );

vector<vector<Point> > contours;
vector<Vec4i> heirarchy;
vector<Point2i> center;
vector<int> radius;


    cv::findContours( cameraFrame.clone(), contours, heirarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE);

size_t count = contours.size();


for( int i=0; i<count; i++)
{
    cv::Point2f c;
    float r;
    cv::minEnclosingCircle( contours[i], c, r);

    //if (!enableRadiusCulling || r >= minTargetRadius)
    //{
        center.push_back(c);
        radius.push_back(r);
    //}
}

size_t count2 = center.size();
cv::Scalar red(255,0,0);

int max = 0;
int maxi = 0;
for( int i = 0; i < count2; i++)
{
  // cv::circle(cameraFrameOrigin, center[i], radius[i], red, 3);
  if(radius[i] >= max){
    maxi = i;
    max = radius[i];
  }
}
      if(!center.empty() && radius.size() == center.size() && maxi < center.size()){
        cout << "center[" << maxi << "] : " << center[maxi].x <<   endl;
        cv::circle(cameraFrameOrigin, center[maxi], max, red, 3);

        if(center[maxi].x < cameraFrame.cols/3){
          if(center[maxi].y < cameraFrame.rows/3){
            cout << "Avancer gauche" << endl;
						if(Sumo)
							Sumo->move(10,-20);

          }
          else if(center[maxi].y < 2*cameraFrame.rows/3){
            cout << "Gauche" << endl;
						if(Sumo)
							Sumo->move(10, -90);
          }
          else if(center[maxi].y < cameraFrame.rows){
            cout << "Reculer gauche" << endl;
						if(Sumo)
							Sumo->move(-10, 20);

          }
        }
        else if(center[maxi].x < 2*cameraFrame.cols/3){
          if(center[maxi].y < cameraFrame.rows/3){
            cout << "Avancer" << endl;
						if(Sumo)
							Sumo->move(15, 0);
          }
          else if(center[maxi].y < 2*cameraFrame.rows/3){
            cout << "Arrêt" << endl;
						if(Sumo)
							Sumo->move(0,0);
          }
          else if(center[maxi].y < cameraFrame.rows){
            cout << "Reculer" << endl;
						if(Sumo)
							Sumo->move(-30,0);
          }
        }
        else if(center[maxi].x < cameraFrame.cols){
          if(center[maxi].y < cameraFrame.rows/3){
            cout << "Avancer droite" << endl;
						if(Sumo)
							Sumo->move(10, 20);
          }
          else if(center[maxi].y < 2*cameraFrame.rows/3){
            cout << "Droite" << endl;
						if(Sumo)
							Sumo->move(10, 90);
          }
          else if(center[maxi].y < cameraFrame.rows){
            cout << "Reculer droite" << endl;
						if(Sumo)
							Sumo->move(-10, -90);
          }
        }


      }else{
				cout << "Arret" << endl;
				if(Sumo)
					Sumo->move(0,0);
			}

    //cout << "max : " << max << endl;
    //cout << "maxi: " << maxi << endl;
    //cout << "rows" << cameraFrame.rows << endl;
    //cout << "cols" << cameraFrame.cols << endl;
    imshow("Camera", cameraFrameOrigin);
		imshow("Result", cameraFrame);



      //imshow("result", dest);
    if(waitKey(30)>=0)
      break;
    }

    return 0;
}
