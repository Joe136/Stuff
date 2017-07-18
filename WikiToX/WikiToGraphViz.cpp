
//---------------------------Includes----------------------------------------------//
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>



//---------------------------Defines-----------------------------------------------//
#define _S std::string()+
;



//---------------------------Struct Node-------------------------------------------//
struct Node {
   std::string                        value;
   std::map<std::string,std::string>  opts;
   Node                              *next  = nullptr;
   Node                              *child = nullptr;

   Node () {}
   Node (const std::string &s) : value (s) {}
   Node (const std::string &s, const std::map<std::string,std::string> &o) : value (s), opts (o) {}

   ~Node () {
      if (next)  delete next;
      if (child) delete child;
   }
};//end Struct



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



//---------------------------Typedefs----------------------------------------------//
typedef std::list<std::string> LinesList;



//---------------------------Forward Declarations----------------------------------//
bool        addFilename          (std::list<std::string> &filenames, const std::string &name);
std::string trim                 (const std::string& str);
bool        getLine              (std::istream &in, std::string &out);
ItemDef     getListDepth         (const std::string &s);
bool        renameFile           (const std::string &oldName, const std::string &newName);
bool        extendBlock          (std::istream &in, std::ostream &out);
void        extendBlockRecursive (LinesList::iterator &it, const LinesList::iterator &end, Node *root, int32_t depth, Node *top);
void        generateListGraphViz (Node *root, std::ostream &out);
Node        processLine          (const std::string &line);
std::vector<std::string> split   (const std::string &str, const std::string &delim);
std::string split (const std::string &str, const std::string &delim, const int32_t num);



//---------------------------Static Variables--------------------------------------//
bool s_cleanup = false;



//---------------------------Start Main--------------------------------------------//
int main (int argc, char *argv[]) {
   if (argc < 2) {
      printf ("WikiToGraphViz <file> [<file> ...]\n");
      return 0;
   }

   std::list<std::string> filenames;
   bool                   recursive = false;
   bool                   genNeato  = false;


   for (int32_t i = 1; i < argc; ++i) {
      std::string arg = argv[i];

      if (arg == "-r") {   //TODO extend
         recursive = true;
         continue;
      } else if (arg == "-c") {   //TODO extend
         s_cleanup = true;
         continue;
      } else if (arg == "--neato") {   //TODO extend
         genNeato = true;
         continue;
      } else if (arg == "-h" || arg == "--help") {   //TODO extend
         printf ("WikiToGraphViz [-c] [-r] [--neato] <file> [<file> ...]\n");
         return 0;
      }

      bool ret = addFilename (filenames, argv[i]);
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

      // Process Wiki list
      extendBlock (file, output);
      modified = true;

      file.close ();

      if (modified) {
         renameFile (filename + ".gv", filename + ".gv.bak");

         std::ofstream out (filename + ".gv", std::ios::out);
         //std::ofstream out (filename + ".tmp", std::ios::out);

         //std::string outS = output.str ();   // We want to remove the last linebreak (will summarize on multiple calls)

         //out << outS.substr (0, outS.size () - 1);
         out << output.str ();

         out.close ();

         if (genNeato) {
            int pid = fork ();
            if (pid == 0) {
               //std::string s = std::string () + "neato -Tsvg -o \"" + filename + ".svg\" \"" + filename + ".gv\"";
               //std::cout << s << std::endl;
               execlp ("neato", "neato", "-Tsvg", "-o", (filename + ".svg").c_str(), (filename + ".gv").c_str(), NULL);
            } else {
               int status;
               waitpid (pid, &status, 0);
            }
         }
      }
   }//end for

   return 0;
}//end Main



//---------------------------Start addFilename-------------------------------------//
// Append Filename if exist, try combinations
static std::string addFilename_suffix[] = {"", ".txt", ".gv"};

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



//---------------------------Start renameFile--------------------------------------//
#ifdef __linux__
bool renameFile (const std::string &oldName, const std::string &newName) {
   return !rename (oldName.c_str (), newName.c_str () );
}//end Fct
#endif



//---------------------------Start extendBlock-------------------------------------//
bool extendBlock (std::istream &in, std::ostream &out) {
   Node        root;
   std::string line;
   LinesList   allLines;

   while (getLine (in, line) ) {
      std::string tline = trim (line);
      if (!tline.empty () )
         allLines.push_back (tline);
   }//end while

   LinesList::iterator it = allLines.begin ();

   if (it == allLines.end () ) return false;


   // Search first non-comment line
   for (; it != allLines.end (); ++it)
      if (it->at (0) != '%')
         break;


   root.value         = *it;   // Header
   root.opts["title"] = *it;


   LinesList::iterator itComment = allLines.begin ();
   while (itComment != it) {
      std::string tline = trim (*itComment);
      size_t pos = tline.find (":");
      std::string key   = trim (tline.substr (1, pos - 1) );
      std::string value = trim (tline.substr (pos + 1) );
      root.opts[key]    = value;
      ++itComment;
   }//end while


   extendBlockRecursive (++it, allLines.end (), &root, 1, &root);

   generateListGraphViz (&root, out);
}//end Fct



//---------------------------Start extendBlockRecursive----------------------------//
void extendBlockRecursive (LinesList::iterator &it, const LinesList::iterator &end, Node *root, int32_t depth, Node *top) {
   Node *current = root->child = new Node ();

   while (it != end) {
      if (it->at (0) == '%') {
         ++it;
         continue;
      }

      ItemDef def = getListDepth (*it);

      if (def.depth > depth) {
         extendBlockRecursive (it, end, current, depth + 1, top);
         //++it;
      } else if (def.depth < depth) {
         return;
      } else {
         std::map<std::string,std::string> gvOpts;

         if (it->find ("###") != std::string::npos) {
            std::vector<std::string> opts = split (it->substr (it->find ("###") + 3), ",");
            for (std::string &o : opts) {
               size_t pos = o.find (":");
               size_t temp5 = 0, temp6 = 0;
               std::string key   = trim (o.substr (0, pos) );
               std::string value = trim (o.substr (pos + 1) );
               std::transform (key.begin(), key.end(), key.begin(), ::tolower);   // Convert key to lower case

               while ( (temp6 = value.find ("$1", temp5) ) != -1) {   // Replace $1 by key
                  std::string s = trim (trim (it->substr (0, it->find ("###") ) ).substr (depth) );
                  value = value.substr (0, temp6) + s + value.substr (temp6 + 2);
                  temp5 = temp6;
               }//end while

               if (value[0] == '<' && value[value.size()-1] == '>')
                  gvOpts[key] = value;
               else
                  gvOpts[key] = _S"\"" + value + "\"";

               if (key == "url")
                  gvOpts["target"] = "\"_blank\"";
            }//end for

            *it = trim (it->substr (0, it->find ("###") ) );
         }

         current = current->next = new Node (trim (it->substr (depth) ), gvOpts);
         ++it;
      }
   }//end for
}//end Fct



//---------------------------Start generateListGraphViz----------------------------//
//TODO implement this
void generateListGraphViz (Node *root, std::ostream &out) {
   out << "digraph \"" << root->opts["title"] << "\" {\n"
       << "graph [root=\"" + root->value + "\",layout=neato,epsilon=0.005,overlap=false," << root->opts["graph"] << "]\n";


   out << "\"" + root->value + "\" [color=red";
   root->opts.erase ("title");
   root->opts.erase ("graph");
   for (auto p : root->opts)
      out << "," << p.first << "=" << p.second;
   out << "]\n";


   std::function<void(Node*)> recursive = [&](Node *root) {
      if (!root) return;
      Node *current = root->child;
      if (!current) return;

      for (Node *child = current->next; child != nullptr; child = child->next) {
         if (child->opts.size () ) {
            out << "\"" + child->value + "\" [";
            for (auto p : child->opts)
               out << p.first << "=" << p.second << ",";
            out << "]\n";
         }
      }//end for

      out << "\"" + root->value + "\" -> { ";

      for (Node *child = current->next; child != nullptr; child = child->next) {
         out << "\"" + child->value + "\" ";
      }//end for

      out << "}\n";

      for (Node *child = current; child != nullptr; child = child->next) {
         recursive (child);
      }//end for
   };

   recursive (root);

   out << "}\n";
}//end Fct



//---------------------------Start processLine-------------------------------------//
Node processLine (const std::string &line) {
   Node node;

   size_t pos = line.find ("###");

   node.name = trim  (line.substr (0, pos) );

   std::vector<std::string> opts = split (line.substr (pos + 3), ",");


   for (std::string &o : opts) {
      size_t pos = o.find (":");
      size_t temp5 = 0, temp6 = 0;

      std::string key   = trim (o.substr (0, pos) );
      std::string value = trim (o.substr (pos + 1) );

      std::transform (key.begin(), key.end(), key.begin(), ::tolower);   // Convert key to lower case

      while ( (temp6 = value.find ("$1", temp5) ) != -1) {   // Replace $1 by key
         std::string s = trim (trim (line.substr (0, line.find ("###") ) ).substr (depth) );
         value = value.substr (0, temp6) + s + value.substr (temp6 + 2);
         temp5 = temp6;
      }//end while

      if (value[0] == '<' && value[value.size()-1] == '>')
         node.opts[key] = value;
      else
         node.opts[key] = _S"\"" + value + "\"";

      if (key == "url")
         node.opts["target"] = "\"_blank\"";
   }//end for

   return node;
}//end Fct



//---------------------------Start split-------------------------------------------//
std::vector<std::string> split (const std::string &str, const std::string &delim) {
   //console.debug (_S"StringProcessing::split(\"" + str + "\", \"" + delim + "\")");

   std::vector<std::string> res;
   size_t beg = 0, pos = 0;

   while (true) {
      pos = str.find (delim, beg);

      if (pos == std::string::npos) {
         res.push_back (str.substr (beg) );
         break;
      }

      res.push_back (str.substr (beg, pos - beg) );
      beg = pos + 1;
   }//end while

   return res;
}//end Fct



std::string split (const std::string &str, const std::string &delim, const int32_t num) {
   //console.debug (_S"StringProcessing::split(\"" + str + "\", \"" + delim + "\", <num>)");

   std::string res;
   size_t beg = 0, pos = 0, curr = 0;

   while (true) {
      pos = str.find (delim, beg);

      if (pos == std::string::npos) {
         if (curr == num)
            res = str.substr (beg);
         break;
      }

      if (curr == num)
         res = str.substr (beg, pos - beg);
      beg = pos + 1;
      ++curr;
   }//end while

   return res;
}//end Fct
