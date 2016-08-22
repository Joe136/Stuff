
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




//---------------------------Struct ItemDef----------------------------------------//
struct ItemDef {
   enum Type {
      ID_Item = 0,
      ID_Enum = 1
   };//end Enum

   int32_t     depth;
   Type        type[33];
   std::string options;
   std::string item;
};//end Struct



//---------------------------Forward Declarations----------------------------------//
bool        addFilename        (std::list<std::string> &filenames, const std::string &name);
std::string trim               (const std::string& str);
bool        getLine            (std::istream &in, std::string &out);
ItemDef     getListDepth       (const std::string &s);
void        generateList       (std::list<std::string> &block, std::ostream &out);
bool        renameFile         (const std::string &oldName, const std::string &newName);
void        extendBlockComment (std::istream &in, std::ostream &out);
void        extendBlockCommand (std::istream &in, std::ostream &out);



//---------------------------Static Variables--------------------------------------//
bool s_cleanup = false;



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
      } else if (arg == "-c") {   //TODO extend
         s_cleanup = true;
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



//---------------------------Start getListDepth------------------------------------//
ItemDef getListDepth (const std::string &s) {
   ItemDef ret = {0};

   for (int32_t i = 0; i < s.size (); ++i) {
      if (s.at(i) == '-') {
         ret.type[ret.depth] = ItemDef::ID_Item;
         ++ret.depth;
      } else if (s.at(i) == '#') {
         ret.type[ret.depth] = ItemDef::ID_Enum;
         ++ret.depth;
      } else
         break;
   }

   return ret;
}//end Fct



//---------------------------Start extendBlockComment------------------------------//
#ifdef __linux__
bool renameFile (const std::string &oldName, const std::string &newName) {
   return !rename (oldName.c_str (), newName.c_str () );
}//end Fct
#endif



//---------------------------Start generateList------------------------------------//
void generateList (std::list<std::string> &block, std::ostream &out) {
   std::string tline;
   int32_t     spaces = 3;
   int32_t     depth  = 0;
   bool        ItemOptChanged = false;
   char        buffer[100];
   ItemDef     defaults[33];
   std::string ItemOpt;


   out << "%begin wikilist\n";


   for (std::string &s : block) {
      tline      = trim (s);

      if (tline[0] == '%') {
         if (tline.find ("%option") == 0) {
            std::string opt = trim (tline.substr (7) );
            char num[3] = { opt[0], opt[1], 0 };
            if (num[0] < '0' || num[0] > '9') { num[0] = '1'; num[1] = 0; }
            if (num[1] < '0' || num[1] > '9') num[1] = 0;
            uint32_t n = std::stoi (num) - 1;
            if (n < 33) defaults[n].options = trim (opt.substr (1 + (num[1] != 0) ) );

         } else if (tline.find ("%item") == 0) {
            ItemOpt = trim (tline.substr (5) );
            ItemOptChanged = true;
         }

         continue;
      }

      ItemDef id = getListDepth (tline);

      if (id.depth == 0) {
         out << tline << "\n";
         continue;
      }


      while (id.depth > depth) {
         memset (buffer, ' ', depth * spaces); buffer[depth * spaces] = 0;
         out << buffer << "\\begin{itemize}";
         if (defaults[depth].options.size () )
            out << "[" << defaults[depth].options << "]";
         out << "\n";
         ++depth;
      }

      while (id.depth < depth) {
         --depth;
         memset (buffer, ' ', depth * spaces); buffer[depth * spaces] = 0;
         out << buffer << "\\end{itemize}\n";
         defaults[depth].item.clear ();
      }

      if (ItemOptChanged) {
         std::swap (defaults[id.depth-1].item, ItemOpt);
         ItemOptChanged = false;
      }

      memset (buffer, ' ', depth * spaces); buffer[depth * spaces] = 0;
      out << buffer << "\\item";
      if (defaults[id.depth-1].item.size () )
         out << "[" <<  defaults[id.depth-1].item << "]";
       out << " " << trim (tline.substr (id.depth) ) << "\n";
   }//end for

   while (0 < depth) {
      --depth;
      memset (buffer, ' ', depth * spaces); buffer[depth * spaces] = 0;
      out << buffer << "\\end{itemize}\n";
   }

   out << "%end wikilist\n";
}//end Fct



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
         block.push_back (line.substr (1) );
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

   if (!s_cleanup)
      generateList (block, out);

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

   if (!s_cleanup)
      generateList (block, out);

   if (eline.size () )
      out << eline;
}//end Fct
