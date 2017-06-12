/*

*/


#pragma once

//---------------------------Includes----------------------------------------------//
#include <cstdint>
#include <ctime>
#include <stdlib.h>
#include <string>



//---------------------------Namespace utils---------------------------------------//
namespace utils {



//---------------------------Class GUID--------------------------------------------//
class GUID {
public:// Constructors
   GUID () { guid.l[0] = 0; guid.l[1] = 0; }

   GUID (const uint64_t l[2]) { guid.l[0] = l[0]; guid.l[1] = l[1]; }
   GUID (const uint32_t i[4]) { guid.i[0] = i[0]; guid.i[1] = i[1]; guid.i[2] = i[2]; guid.i[3] = i[3]; }

   GUID (const std::string &s) {   //FIXME last 4bit not recognized
      if (s == "new")
         generate ();
      else
         sscanf (s.c_str (), s_pGUIDformat,
                       &guid.c[0], &guid.c[1], &guid.c[2],  &guid.c[3],  &guid.c[4],  &guid.c[5],  &guid.c[6],  &guid.c[7],
                       &guid.c[8], &guid.c[9], &guid.c[10], &guid.c[11], &guid.c[12], &guid.c[13], &guid.c[14], &guid.c[15]);
   }//end Constructor


public:// Functions
   void generate () {
      for (int8_t i = 0; i < 8; ++i)
         guid.s[i] = rand () % 65536;
   }//end Fct


   std::string str () const {
      return operator std::string ();
   }//end Fct


public:// Operators
   operator std::string () const {
      char s[37];
      snprintf (s, 37, s_pGUIDformat,
                       guid.c[0], guid.c[1], guid.c[2],  guid.c[3],  guid.c[4],  guid.c[5],  guid.c[6],  guid.c[7],
                       guid.c[8], guid.c[9], guid.c[10], guid.c[11], guid.c[12], guid.c[13], guid.c[14], guid.c[15]);
      return std::string (s);
   }//end Fct

   operator bool () const noexcept {
      return (guid.l[0] != 0 || guid.l[1] != 0);
   }//end Fct

   bool operator! () const noexcept {
      return (guid.l[0] == 0 && guid.l[1] == 0);
   }//end Fct

   bool operator== (const GUID &other) const noexcept {
      return (guid.l[0] == other.guid.l[0] && guid.l[1] == other.guid.l[1]);
   }//end Fct

   bool operator< (const GUID &other) const noexcept {
      // Search for the first different pair, then check if smaller
      for (int8_t i = 0; i < 16; ++i) {
         if (guid.c[i] == other.guid.c[i]) continue;
         return (guid.c[i] < other.guid.c[i]);
      }//end for

      return false;   // All paira are identical
   }//end Fct

   bool operator> (const GUID &other) const noexcept {
      // Search for the first different pair, then check if bigger
      for (int8_t i = 0; i < 16; ++i) {
         if (guid.c[i] == other.guid.c[i]) continue;
         return (guid.c[i] > other.guid.c[i]);
      }//end for

      return false;   // All paira are identical
   }//end Fct


protected:// Variables
   static const constexpr char *s_pGUIDformat = "%02hhX%02hhX%02hhX%02hhX-%02hhX%02hhX-%02hhX%02hhX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX";

public:// Variables
   union {
      //uint128_t t;
      uint64_t  l[2];
      uint32_t  i[4];
      uint16_t  s[8];
      uint8_t   c[16];
   } guid;

};//end Class

};//end namespace



//---------------------------Start Operators---------------------------------------//
std::string operator+ (std::string s, utils::GUID guid) {
   return s + (std::string)guid;
}//end Fct
