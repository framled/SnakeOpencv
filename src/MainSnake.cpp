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
unsigned long length = 0;
int i = 0;
string PUT_POINTS_WIN = "put points";
string SNAKE_WIN = "snake";
string NORMAL_WINDOW = "normal window";
CvPoint* points;


void printUsage(){
  cout << "The options available are: "<< endl
        <<"-r [file path]           Open file that contains a point with cvs, x,y one for line" << endl
        <<"-w [file path]           write file the snake control points x,y one for line" << endl
        <<"-i [image path]          Open the image" << endl
        <<"-a [alpha]               Set alpha value (default 0.5). Elastic properties" << endl
        <<"-b [beta]                Set beta value (default 0.5). Curvature properties" << endl
        <<"-g [gamma]               Set gamma value (default 0.9). Extrinsic properties" << endl
        <<"-e [epsilon]             Set epsilon value" << endl
        <<"-h                       Print this usage" << endl;
}
void snake(Mat& image, CvPoint* ptr_points, float &alpha, float &beta, float &gamma);
void mouseHandler(int event, int x, int y, int flags, void* para);
void putPoints(Mat& img);
void showSnake(Mat& img);
void savePoints(string path);
void readFile(string path);
void newmannBoundaryCondition(Mat& img);
void writeImage(int state , void* pointer);

int main(int argc, char** argv){

    if(pcl::console::find_argument(argc,argv,"-h") >= 0 || argc == 1){
      printUsage();
      return 0;
    }
    namedWindow(NORMAL_WINDOW,WINDOW_NORMAL);
    //CODE TO LOADING IMAGE
    Mat frame, copy;
    if(pcl::console::find_argument(argc,argv,"-i") >= 0){
      //"../resources/fighter.jpg"
      frame = imread (argv[pcl::console::find_argument(argc,argv,"-i")+1], CV_LOAD_IMAGE_GRAYSCALE);
      if(frame.empty()){
        cout << "Error : Image cannot be loaded..!!" << endl;
        system("pause");
        return -1;
      }
      copy = Mat::zeros( frame.size(), frame.type());
      frame.copyTo(copy);
    }else{
      cout << "Error: no image to load" << endl;
      system("pause");
      return -1;
    }

    if(pcl::console::find_argument(argc,argv,"-r") >= 0){
      //"../resources/points_before.txt"
      readFile(argv[pcl::console::find_argument(argc,argv,"-r")+1]);
    }else{
        newmannBoundaryCondition(copy);
      //putPoints(copy);
      if(pcl::console::find_argument(argc,argv,"-w") >= 0){
        savePoints(argv[pcl::console::find_argument(argc,argv,"-w")+1]);
      }
    }
    copy.release();

    GaussianBlur(frame, copy, Size(9, 9), 0, 0);
    threshold( copy, copy, 40, 255,CV_THRESH_BINARY);


    float alpha, beta, gamma;
    alpha = 0.5;beta = 0.5;gamma = 0.9;
    if(pcl::console::find_argument(argc,argv,"-a") >= 0){
      alpha = stof(argv[pcl::console::find_argument(argc,argv,"-a")+1]);
    }
    if(pcl::console::find_argument(argc,argv,"-b") >= 0){
      beta = stof(argv[pcl::console::find_argument(argc,argv,"-b")+1]);
    }
    if(pcl::console::find_argument(argc,argv,"-g") >= 0){
      gamma = stof(argv[pcl::console::find_argument(argc,argv,"-g")+1]);
    }
    snake(copy, points, alpha, beta, gamma);
    showSnake(frame);
    frame.release();
    return 0;
}

void snake(Mat& image, CvPoint* ptr_points, float &alpha, float &beta, float &gamma)
{
    IplImage copy = image;
    IplImage * image2 = &copy;

    CvSize size; // Size of neighborhood of every point used to search the minimumm have to be odd
    size.width = 5;
    size.height = 5;

    CvTermCriteria criteria;
    criteria.type = CV_TERMCRIT_ITER;  // terminate processing after X iteration
    criteria.max_iter = 10000;
    criteria.epsilon = 0.1;

    cout << "starting snake, with setting: " << endl
        << "alpha " << alpha << endl
        << "beta " << beta << endl
        << "gamma " << gamma << endl;
    cvSnakeImage(image2, points, length, &alpha, &beta, &gamma, CV_VALUE, size, criteria, 0);

    cout << "finishing snake" << endl
        << "showing result" << endl;

}
void showSnake(Mat& image){
    int h = image.size().height;
    int w = image.size().width;
    Mat color(h , w , CV_8UC3, Scalar(255,255,255));
    cv::cvtColor(image, color, cv::COLOR_GRAY2BGR);
    createButton("Save",writeImage,&color,CV_PUSH_BUTTON,1);
    for(int j = 1; j < length ; j++){
        circle(color, *(points+j), 2, Scalar(0,0,255), -1);
    }
    imshow(NORMAL_WINDOW,color);
    if(waitKey(0) == 27){
        destroyWindow(NORMAL_WINDOW.c_str());
    }
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
void readFile(string path){
  ifstream file;
  file.open(path);
  if(file.is_open()){
    string output;
    cout << "open file success" << endl;
    char_separator<char> sep(",");
    cout << "reading file" << endl;
    while(getline(file,output)){

      tokenizer< char_separator<char> > tokens(output, sep);
      boost::tokenizer< boost::char_separator<char> >::iterator beg;
      beg = tokens.begin();
      points[i].x = stoi(*tokens.begin(),0,10);
      points[i].y = stoi(*(++tokens.begin()),0,10);
      i++;
    }
    cout << "closing file" << endl;
    file.close();
  }else{
    cout << "Ho ho!!! errors ocurred when I why try open this file: " << path << endl;
  }
}
void mouseHandler(int event, int x, int y, int flags, void* param){
  if(event == CV_EVENT_LBUTTONDOWN){
    if(flags & CV_EVENT_FLAG_CTRLKEY){
        printf("Left button down with CTRL pressed\n");
        Mat& img = *((Mat*) (param));

        Point point(x,y);
        points[i].x = x;
        points[i].y = y;

        cout << points[i].x << points[i].y << endl;
        int color = img.at<uchar>(point);

        cv::circle(img,points[i],2,abs(color-255),-1);
        if(i > 0)
          cv::line(img, points[(i-1)%i],points[i],abs(color-255),2);

        cv::imshow(NORMAL_WINDOW.c_str(),img);
        i++;
    }
  }
}
void putPoints(Mat& img){
  namedWindow(PUT_POINTS_WIN,WINDOW_NORMAL);
  cvSetMouseCallback(PUT_POINTS_WIN.c_str(),mouseHandler,&img);

  imshow(PUT_POINTS_WIN,img);
  if(waitKey(0) == 27 ){
    cout << "Finish points " << endl;
    destroyWindow(PUT_POINTS_WIN.c_str());
    return;
  }
}

void newmannBoundaryCondition(Mat& img)
{
    int width = img.size().width;
    int heigh = img.size().height;
    int margin = 10;
    int step = 10;
    vector<CvPoint> list;
    CvPoint p;
    p.x = margin;
    p.y = margin;
    list.push_back(p);
    while (list.back().y <= heigh - margin){
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
}
