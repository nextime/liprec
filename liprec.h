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

#ifndef __LIPREC_H__
#define __LIPREC_H__


#define LIPREC_OPTIMIZATION_GREY_BASIC       (1)
#define LIPREC_OPTIMIZATION_HSV_BASIC        (2)
#define LIPREC_OPTIMIZATION_GREY_DEEP        (3)
#define LIPREC_OPTIMIZATION_HSV_DEEP         (4) 

#define LIPREC_CONTOUR_THRESHOLD             (1)
#define LIPREC_CONTOUR_AUTOTHRESHOLD         (2)
#define LIPREC_CONTOUR_CANNY                 (3)

#define LIPREC_PLATECON_THRESHOLD            (1)
#define LIPREC_PLATECON_AUTOTHRESHOLD        (2)
#define LIPREC_PLATECON_CANNY                (3)


#ifdef __cplusplus

#include "opencv2/opencv.hpp"
//#include "opencv2/highgui/highgui.hpp"
#include "tesseract/baseapi.h"

#define TESSERACT_PAGETYPE_BLOCK             (tesseract::PSM_SINGLE_BLOCK) 
#define TESSERACT_PAGETYPE_SINGLE_VERT       (tesseract::PSM_SINGLE_BLOCK_VERT_TEXT)
#define TESSERACT_PAGETYPE_SINGLE_CHAR       (tesseract::PSM_SINGLE_CHAR)



namespace liprec
{


   class Plate {

      public:
         cv::Mat contours;
         cv::Mat ocrimage;
         cv::Rect rect;
         cv::String platetxt;
         int confidence;
         //~Plate();
   }; 
   
   class PlatesImage {
   
      public:
         cv::Mat image;
         cv::Mat optimizedimage;
         cv::Mat contours;
         std::vector<Plate> plates;
         //~PlatesImage();

   };


   class LiPRec {

      public:

         LiPRec(int optimization=LIPREC_OPTIMIZATION_GREY_BASIC, 
                int contour=LIPREC_CONTOUR_CANNY, 
                int platecont=LIPREC_PLATECON_THRESHOLD,
                tesseract::PageSegMode pagetype=TESSERACT_PAGETYPE_BLOCK,
                int min_ocr_confidence=33
                );
 
         void optimizeImage(const cv::Mat &inimg, cv::Mat &outimg);
         void detectPlates(cv::Mat &img,  PlatesImage* plates,
                           int min_area=600, int max_area=6000);
         void detectPlates(cv::Mat &img, cv::Mat &optimizedimage, PlatesImage* plates,
                           int min_area=600, int max_area=6000);
         void setThreshold(int min=128, int max=255);
         void setAutothreshold(int size=21);
         void setPlateThreshold(int min, int max=255);
         void setPlateAutothreshold(int size=11);
         void setPerimeterConstant(int val=35);
         virtual ~LiPRec();                // descructor

      private:
         int opt, cont, pcont, min_confidence;
         int thr_min, thr_max, athr_size;
         int thrp_min, thrp_max, athrp_size;
         float perimeter_constant;
         tesseract::TessBaseAPI *OCR;
         tesseract::PageSegMode ocr_ptype;
         void startOCR(tesseract::PageSegMode pagetype);
         void maximizeContrast(cv::Mat &img);
         void extractV(const cv::Mat &inimg, cv::Mat &outimg);
         void _detectPlates(cv::Mat &img, cv::Mat &optimizedimage, PlatesImage* plates,
                           int min_area=600, int max_area=6000);
   };


}

#endif // __cplusplus 

#endif // #ifndef __LIPREC_H
