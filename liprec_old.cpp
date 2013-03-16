/***********************************************************************
    This file is part of LiPRec, License Plate REcognition.

    Copyright (C) 2012 Franco (nextime) Lanza <nextime@nexlab.it>

    LiPRec is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    LiPRec is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with LiPRec.  If not, see <http://www.gnu.org/licenses/>.
************************************************************************/

#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "tesseract/baseapi.h" 



using namespace cv;
using namespace std;
using namespace tesseract;


int high_switch_value = 35;
float perimeter_constant = high_switch_value/1000.0;
int minimum_area= 600.0;
int maximum_area= 6000.0;
int threshold_val = 128;
int threshold2_val = 130;
int athr_size = 21;
int athr2_size = 11;

void switch_callback_h( int position, void* udata ){
   perimeter_constant = position/1000.0;
}

/* NOTE: only for adaptive threshold */
//void athr_callback(int position, void* udata){
// // we need a non-pair value for adaptive threshold!
// if(*((int*)udata)%2==0)
//  *((int*)udata)+=1;
//}


int main(int argc, char* argv[]) {
 
    const char* name_main = "Plate Recognition";
    const char* name_edge = "Edge Window";
    #if defined(__DMDEBUG)
    const char* name_mask = "Mask Window";
    #endif
    const char* name_bwframe = "BW Window";
    const char* name_crop = "CROP Window";
    const char* name_ctrl = "Control Window";


    unsigned int i;
    Mat frame;
    Mat bwframe;
    Mat edge;

    TessBaseAPI *OCR = new TessBaseAPI();
    if(OCR->Init(NULL, "eng")) {
      cout << "Could not initialize tesseract OCR\n";
      return -1;
    }

    if( argc != 2) {
      cout << "You must specify a file to load\n";
      return -1;
    }
     
    VideoCapture cap(argv[1]);
    if(!cap.isOpened()) {
      cout << "Cannot open file " << argv[1] << endl;
      return -1;
    }

    namedWindow(name_main, 0);
    namedWindow(name_edge, 0);
    #if defined(__DMDEBUG)
    namedWindow(name_mask, 0);
    #endif
    namedWindow(name_bwframe, 0);
    namedWindow(name_ctrl, 0);
    //#if defined(__DMDEBUG)
    namedWindow(name_crop, 0);

    namedWindow("AAA", 0);

    //#endif
    createTrackbar( "Contour perimeter", name_ctrl, &high_switch_value, 100, switch_callback_h, NULL );
    createTrackbar( "Min area", name_ctrl, &minimum_area, 100000, NULL, NULL);
    createTrackbar( "Max area", name_ctrl, &maximum_area, 100000, NULL, NULL);
    createTrackbar( "Threshold", name_ctrl, &threshold_val, 255, NULL, NULL);
    //createTrackbar( "thr size", name_ctrl, &athr_size, 255, athr_callback, &athr_size);
    createTrackbar( "Plate Thr", name_ctrl, &threshold2_val, 255, NULL, NULL);
    //createTrackbar( "PThr size", name_ctrl, &athr2_size, 255, athr_callback, &athr2_size);

    waitKey();

    for(;;) {

      //try {
         cap >> frame;
      //} catch(int e) {
      //   frame = imread(argv[1], 3);
      //}
      cvtColor(frame, bwframe, CV_RGB2GRAY);

      imshow(name_bwframe, bwframe); 

      // extract V channel from HSV image
      Mat vframe(frame.rows, frame.cols, CV_8UC1);
      Mat tvframe(frame.rows, frame.cols, CV_8UC3);
      cvtColor(frame, tvframe, CV_RGB2HSV);
      int from_to[] = { 2,0 };
      mixChannels( &tvframe, 1, &vframe,1, from_to, 1);

      // maximize contrast
      // https://github.com/oesmith/OpenANPR/blob/master/anpr/preprocess.py
      Mat el=getStructuringElement(MORPH_ELLIPSE, Size(3,3), Point(1,1));
      Mat bh(vframe.rows, vframe.cols, CV_8UC1);
      Mat th(vframe.rows, vframe.cols, CV_8UC1);
      Mat s1(vframe.rows, vframe.cols, CV_8UC1);
      morphologyEx(vframe, th, MORPH_TOPHAT, el, Point(1,1), 1);
      morphologyEx(vframe, bh, MORPH_BLACKHAT, el, Point(1,1), 1);
      add(vframe, th, s1);
      subtract(s1,bh, vframe);

      // Smooth image to remove rumor
      GaussianBlur(vframe, vframe, Size(5,5), 5, 5, BORDER_DEFAULT);

      imshow("AAA", vframe);

      // apply your filter
      Canny(bwframe, edge, threshold_val, 255);
      //Canny(vframe, edge, threshold_val, 255);
      //threshold( bwframe, edge, threshold_val, 255, CV_THRESH_BINARY );
      //threshold( vframe, edge, threshold_val, 255, CV_THRESH_BINARY );
      //adaptiveThreshold(bwframe, edge, threshold_val, 
      //                  CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, athr_size, 5);
                       // OR CV_ADAPTIVE_THRES_MEAN_C
      //adaptiveThreshold(vframe, edge, threshold_val, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY_INV, athr_size, 9);
      // find the contours
      vector< vector<Point> > contours;

      findContours(edge, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
      vector<double> areas(contours.size());     
      for( i = 0; i < contours.size(); i++) {
         areas[i] = fabs(contourArea(Mat(contours[i])));
         if(areas[i] >= minimum_area && areas[i] <= maximum_area) {

            vector<Point> results;
            approxPolyDP(Mat(contours[i]), results, arcLength(Mat(contours[i]),1)*perimeter_constant,1);
            if (results.size() == 4 && isContourConvex(results)){
               // you could also reuse bwframe here
               Mat mask = Mat::zeros(bwframe.rows, bwframe.cols, CV_8UC1);
 
               // CV_FILLED fills the connected components found
               drawContours(mask, contours, i, Scalar(255,255,255), CV_FILLED);

               // draw contours to cover plate external lines
               drawContours(bwframe, contours, i, Scalar(255,255,255), 2, 2);

               Rect box = boundingRect(Mat(contours[i]));


               // let's create a new image now
               Mat crop(frame.rows, frame.cols, CV_8UC1);
          
               // set background color to black
               //crop.setTo(Scalar(0,0,0));
               crop.setTo(Scalar(0));
          
               // and copy the magic apple
               bwframe.copyTo(crop, mask);
               

               // normalize so imwrite(...)/imshow(...) shows the mask correctly!
               normalize(mask.clone(), mask, 0.0, 255.0, CV_MINMAX, CV_8UC1);

               //rectangle(crop, box, Scalar(0,0,255) ,3);
               Mat roi(crop, box);


               // Image for OCR
               Mat ocrimg(roi.rows, roi.cols, CV_8UC1);
               ocrimg.setTo(Scalar(255));
               roi.copyTo(ocrimg, roi);
               int scalefactor = 1000/ocrimg.cols;
               //cout << "Scale factor: " << scalefactor << endl;;
               resize(ocrimg, ocrimg, Size(0,0), scalefactor, scalefactor, CV_INTER_CUBIC);
               threshold(ocrimg, ocrimg, threshold2_val, 255, CV_THRESH_BINARY );
               //adaptiveThreshold(ocrimg, ocrimg, threshold2_val, 
               //                  CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, athr2_size, 5);
               //Canny(ocrimg, ocrimg, threshold2_val, 255);

               //#if defined(__DMDEBUG)
               imshow(name_crop, ocrimg);
               //#endif

               rectangle(frame, box, Scalar(0,0,255), 3);
               rectangle(bwframe, box, Scalar(0,0,255), 3);
               #if defined(__DMDEBUG)
               imshow(name_mask, mask);
               #endif

               //OCR->TesseractRect(roi.data, 1, roi.step1(), 0,0,roi.cols, roi.rows);
               OCR->SetImage((uchar*)ocrimg.data, ocrimg.size().width, ocrimg.size().height, 
                             ocrimg.channels(), ocrimg.step1());
               OCR->Recognize(0);
               char* detected_text = OCR->GetUTF8Text();
               //cout << "Size text: " << strlen(detected_text) << endl;
               if(strlen(detected_text) > 0) {
                  
                  cout << "License plate number: " << detected_text << endl;
                  //#if !defined(__DMDEBUG)
                  //imshow(name_crop, ocrimg);
                  //#endif
                  imshow(name_main, frame);
                  imshow(name_bwframe, bwframe);
                  imshow(name_edge, edge);
                  imshow(name_ctrl, ocrimg);
                  #if defined(__DMDEBUG)
                  imwrite("antani.jpg", ocrimg);
                  #endif
                  waitKey();
                  //#endif
               }
            }
          } 
       }
       imshow(name_main, frame);
       imshow(name_bwframe, bwframe);
       imshow(name_edge, edge);
       if(waitKey(30) >= 0) break;
    }
    return 0;
}
