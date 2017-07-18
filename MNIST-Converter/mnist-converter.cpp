


#include<cstdio>
#include<fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <opencv2/opencv.hpp>


#define _S std::string()+
;



void    convert    (std::ifstream &file, const std::string &filename);
int32_t readNumber (std::ifstream &file);


static bool verbose = false;


int main (int argc, char *argv[]) {

   if (argc <= 1) {
      printf ("mnist-converter <file>\n");
      return 1;
   }

   for (int32_t i = 1; i < argc; ++i) {
      std::string arg (argv[i]);

      if (arg == "--verbose" || arg == "-v")
         verbose = true;
      else {
         std::ifstream file (arg);
         convert (file, arg + ".dir");
      }
   }


   return 0;
}//end Main



void convert (std::ifstream &file, const std::string &dirname) {
   int32_t magicNumber = readNumber (file);
   int32_t length      = readNumber (file);
   int32_t rows        = readNumber (file);
   int32_t cols        = readNumber (file);

   // file.read ( (char*)&magicNumber, sizeof (int32_t) );
   // file.read ( (char*)&length,      sizeof (int32_t) );
   // file.read ( (char*)&rows,        sizeof (int32_t) );
   // file.read ( (char*)&cols,        sizeof (int32_t) );


   if (verbose)
      printf ("mnist-converter: directory=%s, Magic Number=%i, Length=%i, Rows=%i, Cols=%i\n", dirname.c_str (), magicNumber, length, rows, cols);


   cv::Mat image (rows, cols, CV_8UC1);

   mkdir (dirname.c_str (), 00755);

   char path[11];


   for (int32_t i = 1; i <= length; ++i) {
      file.read ((char*)image.data, rows * cols);
      snprintf (path, 11, "%06i", i);
      cv::imwrite (dirname + "/" + path + ".png", image);
   }//end for 1
}//end Fct



int32_t readNumber (std::ifstream &file) {
   char number[4];
   char sorted[4];

   file.read (number, 4);

   sorted[0] = number[3];
   sorted[1] = number[2];
   sorted[2] = number[1];
   sorted[3] = number[0];

   return *(int32_t*)sorted;
}//end Fct
