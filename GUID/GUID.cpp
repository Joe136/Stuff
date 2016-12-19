/*

*/

//---------------------------Includes----------------------------------------------//
#include "GUID.h"
#include <iostream>



//---------------------------Start Operators---------------------------------------//
std::string operator+ (std::string s, utils::GUID guid) {
   return s + (std::string)guid;
}//end Fct



//---------------------------Start Main--------------------------------------------//
int main (int argc, char *argv[]) {
   std::cout << (std::string)utils::GUID ("new");
}//end Fct
