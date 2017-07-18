


#include<cstdio>
#include<fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <opencv2/opencv.hpp>


#define _S std::string()+
;



void    convert       (std::ifstream &file, const std::string &filename);
void    convertImages (std::ifstream &file, const std::string &filename);
void    convertLabels (std::ifstream &file, const std::string &filename);
int32_t readNumber    (std::ifstream &file);


static bool verbose   = false;
static bool csvLabels = false;


int main (int argc, char *argv[]) {

   if (argc <= 1) {
      printf ("mnist-converter [--verbose|-v] [--csv-labels] [--directory=<dir>] <file>\n");
      return 1;
   }

   std::string dirname;

   for (int32_t i = 1; i < argc; ++i) {
      std::string arg (argv[i]);

      if (arg == "--verbose" || arg == "-v")
         verbose = true;
      else if (arg == "--csv-labels")
         csvLabels = true;
      else if (arg.find ("--directory=") == 0)
         dirname = arg.substr (12);
      else {
         std::ifstream file (arg);
         if (dirname.empty () ) dirname = arg + ".dir";
         convert (file, dirname);
         dirname.clear ();
      }
   }


   return 0;
}//end Main



void convert (std::ifstream &file, const std::string &dirname) {
   int32_t magicNumber = readNumber (file);

   switch (magicNumber) {
   case 2049:
      convertLabels (file, dirname); break;
   case 2051:
      convertImages (file, dirname); break;
   default:
      printf ("mnist-converter: Magic Number '%i' not supported.", magicNumber);
   }//end switch
}//end Main



void convertImages (std::ifstream &file, const std::string &dirname) {
   int32_t length      = readNumber (file);
   int32_t rows        = readNumber (file);
   int32_t cols        = readNumber (file);


   if (verbose)
      printf ("mnist-converter: directory=%s, Magic Number=%i, Length=%i, Rows=%i, Cols=%i\n", dirname.c_str (), 2051, length, rows, cols);


   cv::Mat image (rows, cols, CV_8UC1);

   mkdir (dirname.c_str (), 00755);

   char path[11];


   for (int32_t i = 1; i <= length; ++i) {
      file.read ((char*)image.data, rows * cols);
      snprintf (path, 11, "%06i", i);
      cv::imwrite (dirname + "/" + path + ".png", image);
   }//end for 1
}//end Fct



void convertLabels (std::ifstream &file, const std::string &dirname) {
   int32_t length      = readNumber (file);
   int32_t rows        = readNumber (file);
   int32_t cols        = readNumber (file);


   if (verbose)
      printf ("mnist-converter: directory=%s, Magic Number=%i, Length=%i, Rows=%i, Cols=%i\n", dirname.c_str (), 2049, length, rows, cols);


   char    path[11];
   bool    first = true;
   uint8_t label;

   std::ofstream out;

   if (csvLabels)
      out.open (dirname + ".csv");
   else
      mkdir (dirname.c_str (), 00755);


   for (int32_t i = 1; i <= length; ++i) {
      file.read ( (char*)&label, 1);

      if (csvLabels) {
         if (!first) out << ","; first = false;
         out << (int32_t)label;
      } else {
         snprintf (path, 11, "%06i", i);
         std::ofstream (dirname + "/" + path + ".label") << (int32_t)label;
      }
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
