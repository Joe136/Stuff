
// Add this line to Tex: \newcommand{\wikilist}[1]{\iffalse #1 \fi}

//---------------------------Includes----------------------------------------------//
#include <stdio.h>
#include <string>
#include <list>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <functional>



//---------------------------Forward Declarations----------------------------------//
bool        addFilename        (std::list<std::string> &filenames, const std::string &name);
std::string trim               (const std::string& str);
bool        getLine            (std::istream &in, std::string &out);
int32_t     getDepth           (const std::string &s);
void        writeList          (std::list<std::string> &block, std::ostream &out);
bool        renameFile         (const std::string &oldName, const std::string &newName);
void        extendBlockComment (std::istream &in, std::ostream &out);
void        extendBlockCommand (std::istream &in, std::ostream &out);



//---------------------------Start Main--------------------------------------------//
int main (int argc, char *argv[]) {
   if (argc < 2) {
      printf ("WikiToTex <file> [<file> ...]\n");
      return 0;
   }

   std::list<std::string> filenames;
   bool                   recursive = false;

   std::function<void(const std::string&)> recursiveAddFilename = [&filenames,&recursiveAddFilename](const std::string &f) {
      std::ifstream file (f, std::ios::in);
      std::string   line;

      while (getLine (file, line) ) {
         std::string tline = trim (line);
         std::string name;
         size_t      pos;

         if ( (pos = tline.find ("\\input{") ) != std::string::npos)
            name = tline.substr (pos + 7, tline.size () - (pos+7) - 1);
         else if ( (pos = tline.find ("\\include{") ) != std::string::npos)
            name = tline.substr (pos + 9, tline.size () - (pos+9) - 1);

         if (name.size () && addFilename (filenames, name) )
            recursiveAddFilename (name);
      }//end while

      file.close ();
   };//end Lambda

   for (int32_t i = 1; i < argc; ++i) {
      std::string arg = argv[i];
      if (arg == "-r") {   //TODO extend
         recursive = true;
         continue;
      }

      bool ret = addFilename (filenames, argv[i]);

      if (ret && recursive)
         recursiveAddFilename (argv[i]);
   }//end for

   for (std::string &filename : filenames) {
      std::ifstream      file (filename, std::ios::in);
      std::ostringstream output;
      std::string        line;
      bool               modified = false;

      if (!file.is_open () ) {
         std::cerr << "file \"" << filename << "\" does not exist." << std::endl;
         continue;
      }


      while (getLine (file, line) ) {
         std::string tline = trim (line);
         if (line == "%!wikilist") {
            modified = true;
            extendBlockComment (file, output);
         } else if (tline == "\\wikilist{") {
            modified = true;
            extendBlockCommand (file, output);
         } else
            output << line << "\n";
      }

      file.close ();

      if (modified) {
         renameFile (filename, filename + ".bak");

         std::ofstream out (filename, std::ios::out);
         //std::ofstream out (filename + ".tmp", std::ios::out);

         std::string outS = output.str ();   // We want to remove the last linebreak (will summarize on multiple calls)

         out << outS.substr (0, outS.size () - 1);
      }
   }

   return 0;
}//end Main



//---------------------------Start addFilename-------------------------------------//
// Append Filename if exist, try combinations
static std::string addFilename_suffix[] = {"", ".tex"};

bool addFilename (std::list<std::string> &filenames, const std::string &name) {
//printf ("name: %s\n", name.c_str());
   std::string filename;

   for (std::string &a : addFilename_suffix) {
      std::string s = name + a;

      FILE *f = fopen (s.c_str (), "r");

      if (f) {
         fclose (f);
         filename = s;
         break;
      }
   }//end for

   if (filename.size () ) {
      filenames.push_back (filename);
      return true;
   } else
      return false;
}//end Fct



//---------------------------Start trim--------------------------------------------//
// http://stackoverflow.com/questions/25829143/c-trim-whitespace-from-a-string
std::string trim (const std::string& str) {
   if (!str.size()) return str;
   size_t first = str.find_first_not_of(' ');
   size_t last = str.find_last_not_of(' ');
   return str.substr(first, (last-first+1));
}//end Fct



//---------------------------Start getLine-----------------------------------------//
bool getLine (std::istream &in, std::string &out) {
   if (in.fail() || in.eof() ) {
      out = "";
      return false;
   }

   const int32_t maxLength = 8192;
   char          buffer[maxLength];

   in.getline (buffer, maxLength);

   out = buffer;

   return true;
}//end Fct



//---------------------------Start getDepth----------------------------------------//
int32_t getDepth (const std::string &s) {
   int32_t ret = 0;

   for (int32_t i = 0; i < s.size (); ++i) {
      if (s.at(i) == '-')
         ++ret;
      else
         break;
   }

   return ret;
}//end Fct



//---------------------------Start writeList---------------------------------------//
void writeList (std::list<std::string> &block, std::ostream &out) {
   std::string line;
   int32_t     spaces = 3;
   int32_t     depth  = 0;
   char        buffer[100];

   out << "%begin wikilist\n";

   for (std::string &s : block) {
      line = trim (s);
      int32_t d = getDepth (line);

      if (d == 0) {
         out << line << "\n";
         continue;
      }


      while (d > depth) {
         memset (buffer, ' ', depth * spaces); buffer[depth * spaces] = 0;
         out << buffer << "\\begin{itemize}\n";
         ++depth;
      }

      while (d < depth) {
         --depth;
         memset (buffer, ' ', depth * spaces); buffer[depth * spaces] = 0;
         out << buffer << "\\end{itemize}\n";
      }

      memset (buffer, ' ', depth * spaces); buffer[depth * spaces] = 0;
      out << buffer << "\\item " << trim (line.substr (d) ) << "\n";
   }//end for

   while (0 < depth) {
      --depth;
      memset (buffer, ' ', depth * spaces); buffer[depth * spaces] = 0;
      out << buffer << "\\end{itemize}\n";
   }

   out << "%end wikilist\n";
}//end Fct



//---------------------------Start extendBlockComment------------------------------//
#ifdef __linux__
bool renameFile (const std::string &oldName, const std::string &newName) {
   return !rename (oldName.c_str (), newName.c_str () );
}//end Fct
#endif



//---------------------------Start extendBlockComment------------------------------//
void extendBlockComment (std::istream &in, std::ostream &out) {
   std::string line;
   std::string eline;
   std::list<std::string> block;
   block.push_back ("%!wikilist");
   bool cleanup = false;

   while (getLine (in, line) ) {
      if (cleanup) {
         if (line == "%end wikilist")
            break;
      } else if (line.size () == 0) {
         break;
      } else if (line.at (0) == '%') {
         block.push_back (line);
      } else if (line == "%begin wikilist")
         cleanup = true;
      else {
         eline = line + "\n";
         break;
      }
   }//end while

   for (std::string &s : block)
      out << s << "\n";

   block.pop_front ();   // Remove initialzation line

   writeList (block, out);

   if (eline.size () )
      out << eline;
}//end Fct



//---------------------------Start extendBlockCommand------------------------------//
void extendBlockCommand (std::istream &in, std::ostream &out) {
   std::string line;
   std::string tline;
   std::string eline;
   std::list<std::string> block;
   block.push_back ("\\wikilist{");
   bool cleanup = false;
   bool append  = true;

   while (getLine (in, line) ) {
      tline = trim (line);

      if (cleanup) {
         if (line == "%end wikilist")
            break;
      } else if (append && !tline.size () ) {
      } else if (append && tline.at (0) == '}') {
         append = false;
      } else if (!append && line == "%begin wikilist") {
         cleanup = true;
      } else if (append) {
         block.push_back (line);
      } else {
         eline = line + "\n";
         break;
      }
   }//end while

   block.push_back ("}");

   for (std::string &s : block)
      out << s << "\n";

   block.pop_front ();   // Remove initialzation line
   block.pop_back ();    // Remove closing line

   writeList (block, out);

   if (eline.size () )
      out << eline;
}//end Fct