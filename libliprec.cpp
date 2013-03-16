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

#include "liprec.h"
#include <stdexcept>
#include <string>
#include "opencv2/opencv.hpp"
#ifdef __SHOWIMAGES
   #include "opencv2/highgui/highgui.hpp"
#endif


namespace liprec
{


cv::String Filter(const std::string &to)
{
    cv::String final;
    for(cv::String::const_iterator it = to.begin(); it != to.end(); ++it)
    {
        if((*it >= '0' && *it <= '9') || (*it >= 'A' && *it <= 'Z'))
        {
            final += *it;
        }
    }
    return final;
}


class LiprecException : public std::runtime_error
{
   public:
      LiprecException(const char* except) : runtime_error(except) { }
};

LiPRec::LiPRec(int optimization,
               int contour,
               int platecont,
               tesseract::PageSegMode pagetype,
               int min_ocr_confidence
               )
{
   
   opt=optimization;
   cont=contour;
   pcont=platecont;
   ocr_ptype=pagetype;
   min_confidence=min_ocr_confidence;
   startOCR(pagetype);
   thr_min=128;
   thr_max=255;
   athr_size=21;
   thrp_min=130;
   thrp_max=255;
   athrp_size=11;
   perimeter_constant = 35/1000.0;
   #ifdef __DEBUG
   std::cout << "LiPRec Initialized\n";
   #endif
   #ifdef __SHOWIMAGES
   cv::namedWindow("original", 0);
   cv::namedWindow("edge", 0);
   cv::namedWindow("crop", 0);
   cv::namedWindow("mask", 0);
   cv::namedWindow("optimized", 0);
   cv::namedWindow("ocr", 0);
   cv::waitKey();
   #endif

}

LiPRec::~LiPRec()
{
   if(OCR!=NULL)
   {
      OCR->Clear();
      OCR->End();
      delete OCR;
   }
}

void LiPRec::startOCR(tesseract::PageSegMode pagetype)
{
   #ifdef __DEBUG
   std::cout << "LiPRec startOCR\n";
   #endif

   OCR = new tesseract::TessBaseAPI();
   if(OCR->Init(NULL, NULL, tesseract::OEM_DEFAULT, NULL, 0, NULL, NULL, false)) {
      delete OCR;
      throw LiprecException("Could not initialize tesseract OCR");
   }
   OCR->SetPageSegMode(pagetype);
   OCR->SetVariable("tessedit_char_whitelist", "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
}


void LiPRec::maximizeContrast(cv::Mat &img)
{
   #ifdef __DEBUG
   std::cout << "LiPRec maximizeContrast\n";
   #endif

   cv::Mat el=getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3,3), cv::Point(1,1));
   cv::Mat bh(img.rows, img.cols, CV_8UC1);
   cv::Mat th(img.rows, img.cols, CV_8UC1);
   cv::Mat s1(img.rows, img.cols, CV_8UC1);
   cv::morphologyEx(img, th, cv::MORPH_TOPHAT, el, cv::Point(1,1), 1);
   cv::morphologyEx(img, bh, cv::MORPH_BLACKHAT, el, cv::Point(1,1), 1);
   cv::add(img, th, s1);
   cv::subtract(s1,bh, img);

}

void LiPRec::extractV(const cv::Mat &inimg, cv::Mat &outimg)
{
   #ifdef __DEBUG
   std::cout << "LiPRec extractV\n";
   #endif

   cv::Mat tvframe(inimg.rows, inimg.cols, CV_8UC3);
   cvtColor(inimg, tvframe, CV_RGB2HSV);
   int from_to[] = { 2,0 };
   mixChannels( &tvframe, 1, &outimg,1, from_to, 1);
}

void LiPRec::optimizeImage(const cv::Mat &inimg, cv::Mat &outimg)
{
   #ifdef __DEBUG
   std::cout << "LiPRec optimizeImage\n";
   #endif

   switch(opt)
   {
      case LIPREC_OPTIMIZATION_GREY_BASIC:
        cvtColor(inimg, outimg, CV_RGB2GRAY);
        break;

      case LIPREC_OPTIMIZATION_HSV_BASIC:
        extractV(inimg, outimg);
        break;

      case LIPREC_OPTIMIZATION_GREY_DEEP:
        cvtColor(inimg, outimg, CV_RGB2GRAY);
        maximizeContrast(outimg);
        // Smooth image to remove rumor...
        cv::GaussianBlur(outimg, outimg, cv::Size(5,5), 5, 5, cv::BORDER_DEFAULT);
        break;

      case LIPREC_OPTIMIZATION_HSV_DEEP:
        extractV(inimg, outimg);
        maximizeContrast(outimg);
        // Smooth image to remove rumor...
        cv::GaussianBlur(outimg, outimg, cv::Size(5,5), 5, 5, cv::BORDER_DEFAULT);
        break;
   }
}

void LiPRec::setPerimeterConstant(int val)
{
   #ifdef __DEBUG
   std::cout << "LiPRec setPerimeterConstant\n";
   #endif

   perimeter_constant = val/1000.0;
}

void LiPRec::setThreshold(int min, int max)
{
   #ifdef __DEBUG
   std::cout << "LiPRec setThreshold\n";
   #endif

   thr_min=min;
   thr_max=max;
}

void LiPRec::setAutothreshold(int size)
{
   #ifdef __DEBUG
   std::cout << "LiPRec setAutoThreshold\n";
   #endif

   athr_size=size;
}

void LiPRec::setPlateThreshold(int min, int max)
{
   #ifdef __DEBUG
   std::cout << "LiPRec setPlateThreshold\n";
   #endif

   thrp_min=min;
   thrp_max=max;
}

void LiPRec::setPlateAutothreshold(int size)
{
   #ifdef __DEBUG
   std::cout << "LiPRec setAutoPlatethreshold\n";
   #endif

   athrp_size=size;
}


void LiPRec::detectPlates(cv::Mat &img, PlatesImage* plates,
                         int min_area, int max_area)
{

   #ifdef __DEBUG
   std::cout << "LiPRec detectPlates no optimized\n";
   #endif

   cv::Mat optimized(img.rows, img.cols, CV_8UC1);
   optimizeImage(img, optimized);
   _detectPlates(img, optimized, plates, min_area, max_area);
}

void LiPRec::detectPlates(cv::Mat &img, cv::Mat &optimizedimage, PlatesImage* plates, 
                                                   int min_area, int max_area)
{
   #ifdef __DEBUG
   std::cout << "LiPRec detectPlates optimized\n";
   #endif

   _detectPlates(img, optimizedimage, plates, min_area, max_area);
}


void LiPRec::_detectPlates(cv::Mat &img, cv::Mat &optimizedimage, PlatesImage* plates,
                         int min_area, int max_area)
{
   #ifdef __DEBUG
   std::cout << "LiPRec detectPlates real\n";
   #endif
   #ifdef __SHOWIMAGES
   imshow("original",img);
   #endif



   cv::Mat edge;

   switch(cont)
   {
      case LIPREC_CONTOUR_THRESHOLD:
         cv::threshold( optimizedimage, edge, thr_min, thr_max, CV_THRESH_BINARY );
         break;

      case LIPREC_CONTOUR_AUTOTHRESHOLD:
         cv::adaptiveThreshold(optimizedimage, edge, thr_min, 
                  CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY_INV, athr_size, 5);
         break;

      case LIPREC_CONTOUR_CANNY:
      default:
         cv::Canny(optimizedimage, edge, thr_min, thr_max);

   }
   img.copyTo(plates->image);
   optimizedimage.copyTo(plates->optimizedimage);
   
   std::vector< std::vector<cv::Point> > contours;
   cv::findContours(edge, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
   #ifdef __SHOWIMAGES
   imshow("edge",edge);
   #endif
   std::vector<double> areas(contours.size());
   unsigned int i;
   for( i = 0; i < contours.size(); i++) {
      areas[i] = std::fabs(cv::contourArea(cv::Mat(contours[i])));
      if(areas[i] >= min_area && areas[i] <= max_area) {
         std::vector<cv::Point> results;
         cv::approxPolyDP(cv::Mat(contours[i]), results, 
               cv::arcLength(cv::Mat(contours[i]),1)*perimeter_constant,1);
         if (results.size() == 4 && cv::isContourConvex(results)) {
            #ifdef __DEBUG
            std::cout << "LiPRec Possible plate found\n";
            #endif

            // Prepare a mask image
            cv::Mat mask = cv::Mat::zeros(img.rows, img.cols, CV_8UC1);  
            // draw the contours filled on the mask
            cv::drawContours(mask, contours, i, cv::Scalar(255,255,255), CV_FILLED);
            // draw contours on the optimized image  to remove external lines
            cv::drawContours(optimizedimage, contours, i, cv::Scalar(255,255,255), 2, 2);
            // get rectangle of the possible plate
            cv::Rect box = cv::boundingRect(cv::Mat(contours[i]));
            // prepare a new image to copy the masked rectangle
            cv::Mat crop(img.rows, img.cols, CV_8UC1);
            crop.setTo(cv::Scalar(0));
            // copy the masked rectangle
            optimizedimage.copyTo(crop, mask);

            #ifdef __SHOWIMAGES
              imshow("optimized",optimizedimage);
              imshow("crop", crop);
            #endif

            // back to the original optimized, so, we dont get the white contour
            plates->optimizedimage.copyTo(optimizedimage);

            // set the region of interest where the masked rectangle is
            cv::Mat roi(crop, box);
            // prepare an image for the OCR with size equal to the rectangle
            cv::Mat ocrimg(roi.rows, roi.cols, CV_8UC1);
            ocrimg.setTo(cv::Scalar(255));
            roi.copyTo(ocrimg, roi);
            // we need to resize the image for the OCR...
            if(ocrimg.rows < 150) {
               int scalefactor = 150/ocrimg.rows;
               cv::resize(ocrimg, ocrimg, cv::Size(0,0), scalefactor, scalefactor, CV_INTER_CUBIC);
            }
            // and then get a thresholded image to pass to OCR..
            switch(pcont)
            {
               case LIPREC_PLATECON_AUTOTHRESHOLD:
                  cv::adaptiveThreshold(ocrimg, ocrimg, thrp_min, 
                      CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, athrp_size, 5);
                  break;

               case LIPREC_PLATECON_CANNY:
                  cv::Canny(ocrimg, ocrimg, thrp_min, thrp_max);                 
                  break;
      
               case LIPREC_PLATECON_THRESHOLD:
               default:
                  cv::threshold(ocrimg, ocrimg, thrp_min, thrp_max, CV_THRESH_BINARY );
            }
            // NOTE: using OCR this way make the library work
            // only with plates that uses occidental english alphabet and arabic numbers...
            #ifdef __SHOWIMAGES
               imshow("ocr",ocrimg);
               imshow("mask",mask);
            #endif

            OCR->SetImage((uchar*)ocrimg.data, ocrimg.size().width, ocrimg.size().height,
                          ocrimg.channels(), ocrimg.step1());
            OCR->Recognize(0);
            // XXX Gestire il caso in cui c'e' pagetype a single char
            char* detected_text = OCR->GetUTF8Text(); 
            int confidence = OCR->MeanTextConf();
            //cout << "Size text: " << strlen(detected_text) << endl;
            if(strlen(detected_text) > 0 && confidence>=min_confidence) {
               cv::String clean_text;
               clean_text = Filter(cv::String(detected_text));
               if(clean_text.size() > 0)
               {

                  // Hey! maybe we have a plate!
                  // XXX TODO: here we need to write a parser that try to recognize
                  //           only valid plates schemas. To do that probably
                  //           we need also a database of various plates schema in 
                  //           used around the world.
               
                  #ifdef __DEBUG
                  std::cout << "LiPRec FOUND PLATE: " << clean_text << std::endl;
                  std::cout << "Confidence level: " << confidence << std::endl;
                  std::cout << "PLATE lenght: " << clean_text.size() << std::endl;
                     #ifdef __SHOWIMAGES
                     cv::waitKey();
                     #endif
                  #endif

                  Plate plate;
                  ocrimg.copyTo(plate.ocrimage);
                  img.copyTo(plate.contours);
                  rectangle(plate.contours, box, cv::Scalar(0,0,255), 3);
                  plate.rect = box;
                  plate.platetxt = clean_text;
                  plate.confidence = confidence;
                  plates->plates.push_back(plate);
                  rectangle(plates->contours, box, cv::Scalar(0,0,255), 3);
               }
            }           
         }
      }
   }

}



} // end namespace liprec

