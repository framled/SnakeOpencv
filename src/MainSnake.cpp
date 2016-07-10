#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <boost/tokenizer.hpp>
#include <pcl/console/parse.h>
#include <opencv2/opencv.hpp>
#include <opencv/cv.hpp>
#include <opencv2/legacy/legacy.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <qt5/QtWidgets/QApplication>
#include <qt5/QtWidgets/QFileDialog>

using namespace std;
using namespace cv;
using namespace boost;

struct RGB {
    uchar blue;
    uchar green;
    uchar red;
};

Mat src;
unsigned long length = 0;
int i = 0;
int alpha = 1, beta = 4, gama = 9;
int max_alpha = 10, max_beta = 10, max_gamma = 10;
int sigma = 0, max_sigma = 10;

string NORMAL_WINDOW = "normal window";
CvPoint* points;


void printUsage(){
  cout << "The options available are: "<< endl
        <<"-w [file path]           write file the snake control points x,y one for line" << endl
        <<"-i [image path]          Open the image" << endl
        <<"-h                       Print this usage" << endl;
}
void snake(int _, void* p);
void showSnake();
CvPoint* newmannBoundaryCondition(int width, int height);
void writeImage(int state , void* pointer);

int main(int argc, char** argv){

    if(pcl::console::find_argument(argc,argv,"-h") >= 0 || argc == 1){
      printUsage();
      return 0;
    }

    namedWindow(NORMAL_WINDOW,WINDOW_NORMAL);
    //CODE TO LOADING IMAGE
    if(pcl::console::find_argument(argc,argv,"-i") >= 0){
      //"../resources/fighter.jpg"
      src = imread (argv[pcl::console::find_argument(argc,argv,"-i")+1], CV_LOAD_IMAGE_COLOR);
      if(src.empty()){
        cout << "Error : Image cannot be loaded..!!" << endl;
        system("pause");
        return -1;
      }
    }else{
      cout << "Error: no image to load" << endl;
      system("pause");
      return -1;
    }
    Mat dst;
    dst = Mat::zeros( src.size(), src.type());
    src.copyTo(dst);
    cv::cvtColor(dst, dst, cv::COLOR_BGR2GRAY);

    createTrackbar("Alpha:\n", NORMAL_WINDOW, &alpha, max_alpha, snake, &dst);
    createTrackbar("Beta:\n", NORMAL_WINDOW, &beta, max_beta, snake ,&dst);
    createTrackbar("Gamma:\n", NORMAL_WINDOW, &gama, max_gamma, snake ,&dst);
    createTrackbar("Sigma:\n", NORMAL_WINDOW, &sigma, max_sigma, snake ,&dst);

    snake(0, &dst);
    if(waitKey(0) == 27){
        destroyWindow(NORMAL_WINDOW.c_str());
    }
    src.release();

    return 0;
}

void snake(int _, void* p)
{
    Mat* ptr_dst = (Mat*) p;

    cout << "Start Filter " << endl;
    float s = (float) sigma/10.0;
    GaussianBlur(*ptr_dst, *ptr_dst, Size(9, 9), s, s);
    //threshold( *ptr_dst, *ptr_dst, 0, 255,THRESH_BINARY|THRESH_OTSU);
    cout << "End Filter " << endl;

    IplImage* img = new IplImage(*ptr_dst);

    CvSize size; // Size of neighborhood of every point used to search the minimumm have to be odd
    size.width = 3;
    size.height = 3;

    CvTermCriteria criteria;
    criteria.type = CV_TERMCRIT_ITER;  // terminate processing after X iteration
    criteria.max_iter = 100;
    criteria.epsilon = 0.1;

    cout << "Start Neumann bound condition " << endl;
    points = newmannBoundaryCondition(img->width, img->height);
    cout << "End Neumann bound condition " << endl;

    float a = (float)alpha/10.0 ,b = (float)beta/10.0 ,g = (float) gama/10.0;

    cout << "alpha: " << a << " , beta: " << b << " , gamma: " << g << endl;

    cout << "Start Snake with" << endl;
    cvSnakeImage(img, points, length, (float*)&a, (float*)&b , (float*)&g , CV_VALUE, size, criteria, 0);
    cout << "End Snake" << endl;

    showSnake();
}

void showSnake(){
    createButton("Save",writeImage,&src,CV_PUSH_BUTTON,1);
    Mat temp;
    temp = Mat::zeros( src.size(), src.type());
    src.copyTo(temp);
    for(int j = 1; j < length ; j++){
        circle(temp, *(points+j), 4, Scalar(0,0,255), -1);
    }
    imshow(NORMAL_WINDOW,temp);
}


void writeImage(int state, void* pointer){
    std::vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
    compression_params.push_back(100);
    Mat* img = (Mat*) pointer;
    QString fileName = QFileDialog::getSaveFileName(0,"Save file",QDir::currentPath(),NULL,new QString("Images (*.png *.xpm *.jpg)"));
    std::cout << fileName.toStdString() << std::endl;
    imwrite(fileName.toStdString(), *img, compression_params);
}

void savePoints(string path){
  FILE * pFile;
    pFile = fopen (path.c_str(),"w");
    if (pFile!=NULL)
    {
      for(int j = 0; j< length ; j++){
          string txt = to_string((points+j)->x) + "," + to_string((points+j)->y) + "\n";
          fputs(txt.c_str(),pFile);
        }
      fclose (pFile);
    }

}

CvPoint* newmannBoundaryCondition(int width, int height)
{
    int margin = 10;
    int step = 1;
    vector<CvPoint> list;
    CvPoint p;
    p.x = margin;
    p.y = margin;
    list.push_back(p);
    while (list.back().y <= height - margin){
        CvPoint point;
        point.x = margin;
        point.y = list.back().y + step;
        list.push_back(point);
    }
    while (list.back().x <= width - margin){
        CvPoint point;
        point.x = list.back().x + step;
        point.y = list.back().y;
        list.push_back(point);
    }
    while (list.back().y >= 0 + margin){
        CvPoint point;
        point.x = list.back().x;
        point.y = list.back().y - step;
        list.push_back(point);
    }
    while (list.back().x >= 0 + margin){
        CvPoint point;
        point.x =  list.back().x - step;
        point.y = list.back().y;
        list.push_back(point);
    }
    points = new CvPoint[list.size()];
    length = list.size();
    std::copy(list.begin(), list.end() , points);
    return points;
}