

//---------------------------Includes----------------------------------------------//
#include <cmath>
#include <cstdio>
#include <fstream>
#include <string>
#include <list>



//---------------------------Forward Declaration-----------------------------------//
std::string trim   (const std::string &str);
std::string beauty (const std::string &str);



typedef std::list<std::string>::iterator Iter;

//---------------------------Start Main--------------------------------------------//
int main () {
   std::ifstream swap ("/proc/swaps");

   std::string line;

   std::list<std::string> file;
   std::list<std::string> type;
   std::list<std::string> size;
   std::list<std::string> used;
   std::list<std::string> prio;

   do {
      std::getline (swap, line);   // Header line

      if (swap.eof () ) break;

      //---------------------Process-----------------------------------------------//

      size_t pos[6]   = {0, 0, 0, 0, 0, line.size ()};
      bool   repeated = false;

      // Walk reverse and search for spaces (get first char of elements)
      for (int32_t n = line.size () - 1, a = 4; n >= 0; --n) {
         if (!repeated && (line[n] == ' ' || line[n] == '\t') ) {
            pos[a] = n + 1;
            --a;
            if (a == 0) break;
            repeated = true;
         } else
            repeated = false;
      }//end for

      file.push_back (trim (line.substr (pos[0], pos[1] - pos[0]) ) );
      type.push_back (trim (line.substr (pos[1], pos[2] - pos[1]) ) );
      size.push_back (trim (line.substr (pos[2], pos[3] - pos[2]) ) );
      used.push_back (trim (line.substr (pos[3], pos[4] - pos[3]) ) );
      prio.push_back (trim (line.substr (pos[4], pos[5] - pos[4]) ) );
   } while (true);

   // Beautyfy sizes
   for (std::string &s : size) s = beauty (s);
   for (std::string &s : used) s = beauty (s);

   // Max string length per type
   size_t len[5] = {0, 0, 0, 0, 0};

   for (std::string &s : file) len[0] = std::max (len[0], s.size () );
   for (std::string &s : type) len[1] = std::max (len[1], s.size () );
   for (std::string &s : size) len[2] = std::max (len[2], s.size () );
   for (std::string &s : used) len[3] = std::max (len[3], s.size () );
   for (std::string &s : prio) len[4] = std::max (len[4], s.size () );

   Iter it_file = file.begin ();
   Iter it_type = type.begin ();
   Iter it_size = size.begin ();
   Iter it_used = used.begin ();
   Iter it_prio = prio.begin ();

   for (int32_t i = 0; i < file.size (); ++i) {
      char fmt[42];
      snprintf (fmt, 42, "%%-%lus   %%-%lus %%-%lus %%-%lus %%-%lus\n", len[0], len[1], len[2], len[3], len[4]);

      printf (fmt, it_file->c_str (), it_type->c_str (), it_size->c_str (), it_used->c_str (), it_prio->c_str ());

      ++it_file;
      ++it_type;
      ++it_size;
      ++it_used;
      ++it_prio;
   }//end for

   //for (auto s : prio)
   //   printf ("%s\n", s.c_str());


   return 0;
}//end Main



//---------------------------Start trim--------------------------------------------//
std::string trim (const std::string &str) {
   size_t beg = 0;
   size_t end = str.size ();

   for (int32_t i = 0; i < str.size (); ++i) {
      if (str[i] == ' ' || str[i] == '\t')
         beg = i;
      else
         break;
   }//end for

   for (int32_t i = str.size () - 1; i >= 0; --i) {
      if (str[i] == ' ' || str[i] == '\t')
         end = i;
      else
         break;
   }//end for

   return str.substr (beg, end - beg);
}//end Fct




//---------------------------Start beauty------------------------------------------//
std::string beauty (const std::string &str) {
   int32_t i = 0;

   try {
      i = std::stoi (str);
   } catch (...) {
      return str;
   }//end catch

   std::string out;

   if (i > 1e9) {
      char num[12];
      snprintf (num, 12, "%.1f", i / 1e9);
      out = std::string () + num + "T";
   } else if (i > 1e6) {
      char num[12];
      snprintf (num, 12, "%.1f", i / 1e6);
      out = std::string () + num + "G";
   } else if (i > 1e3) {
      char num[12];
      snprintf (num, 12, "%.1f", i / 1e3);
      out = std::string () + num + "M";
   } else
      out = str + "K";

   return out;
}//end Fct

