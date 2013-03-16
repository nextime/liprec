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
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "optionparser.h"

using namespace liprec;
using namespace std;
using namespace cv;

enum  optionIndex { OPT_UNKNOWN, OPT_HELP, OPT_DEBUG, OPT_GUI, OPT_PAUSE};
const option::Descriptor usage[] =
 {
  {OPT_UNKNOWN, 0,"", ""    ,option::Arg::None, "USAGE: liprec [options] <video_file|image_file|video uri>\n\n"
                                                 "Options:" },
  {OPT_HELP,    0,"h","help",option::Arg::None, "  -h, --help  \tPrint usage and exit." },
  {OPT_DEBUG,   0,"d","debug",option::Arg::Optional, "  -d[level], --debug[=level]  \tSet debug level."},
  {OPT_GUI,     0,"g","gui",option::Arg::None, "  -g, --gui  \tshow graphic UI." },
  {OPT_PAUSE,   0,"p","",option::Arg::None, "  -p  \tpause video on plate detected\n"},
  {OPT_UNKNOWN, 0,"", ""   ,option::Arg::None, "\nExamples:\n"
                                                 "  liprec -d file1.mjpeg\n"
                                                 "  liprec http://<ip_addr>/img/video.h264\n" 
                                                 "  liprec -g file.jpg\n" },
  {0,0,0,0,0,0}
 };

int main(int argc, char* argv[])
{

   int debug_level=0;
   int use_gui=0;
   int pause=0;
   Mat frame;
   LiPRec plateDetector;

   argc-=(argc>0); argv+=(argc>0); // skip program name argv[0] if present
   option::Stats  stats(usage, argc, argv);
   option::Option options[stats.options_max], buffer[stats.buffer_max];
   option::Parser parse(usage, argc, argv, options, buffer);
   if(parse.error())
   {
      cout << "Error parsing options\n";
      return -1;
   }
   
   double imgnum=0;
   #ifdef __DEBUG
   debug_level=1;
   #endif
   #ifdef __SHOWIMAGES
   debug_level=2;
   #endif
   //if( argc != 2) {
   if(parse.nonOptionsCount()<1) {
     cout << "You must specify a file to load\n\n";
     option::printUsage(std::cout, usage);
     return -1;
   }
   
   if (options[OPT_HELP]) {
      option::printUsage(std::cout, usage);
      return 0;
   }

   for (int i = 0; i < parse.optionsCount(); ++i) {
      option::Option& opt = buffer[i];
      switch(opt.index()) {
         case OPT_HELP:
           option::printUsage(std::cout, usage);
           return 0;
         case OPT_DEBUG:
            debug_level=1;
            if(opt.arg && atoi(opt.arg)<3)
            {
               debug_level=atoi(opt.arg);
               if(debug_level > 0)
                  cout << "Debug level is " << debug_level << endl;
                  if(debug_level > 1)
                     use_gui=1;
            }
            break;  
         case OPT_GUI:
            use_gui=1;
            break;
         case OPT_PAUSE:
            pause=1;
            break;
      }
   }

   //VideoCapture cap(argv[1]);
   VideoCapture cap(parse.nonOption(0));
   if(!cap.isOpened()) {
     cout << "Cannot open file " << argv[1] << endl;
     return -1;
   }

   if(use_gui) {
      cv::namedWindow("LiPRec", 0);
   }

   for(;;) {
      //#ifdef __DEBUG
      if(debug_level) {
         imgnum++;
         cout << "Working in frame # " << imgnum << endl;
      }
      if(cap.grab())
         cap.retrieve(frame);
      else
      {
         cout << "Video is over\n";
         cv::waitKey(0);
         break;
      }
      PlatesImage plates;
      //plateDetector.optimizeImage(frame, frame);
      plateDetector.detectPlates(frame, &plates);
      if(debug_level) {
         cout << "Plates vector size: " << plates.plates.size() << endl;
      }
      if(use_gui) {
         cv::imshow("LiPRec", plates.image);
      }  
      if(plates.plates.size() > 0) {
         for(unsigned int i=0;i<plates.plates.size();i++) {
            cout << "** Plates found: " << plates.plates[i].platetxt;
            cout << "   (confidence:" << plates.plates[i].confidence << ")"  << endl;
         }
         if(pause) {
            if(use_gui) {
               cv::waitKey();
            }
            else {  
               std::cout << "Press enter to continue: ";
               std::cin >> pause;
            }
         }
      }
      if(debug_level>1 || use_gui) {
         if(cv::waitKey(30) >= 0) break;
      }
   }
   
   return 0;
}

