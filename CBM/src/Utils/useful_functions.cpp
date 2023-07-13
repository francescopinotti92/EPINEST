//
//  useful_functions.cpp
//  CBM
//
//

#include "useful_functions.hpp"


bool does_path_exist( const std::string &s ) {
  struct stat buffer;
  return ( stat (s.c_str(), &buffer ) == 0 );
}


namespace utils {
   
using std::to_string ;

   std::string to_string(const BirdID& x) {

       return x.toString() ;

   }

}
