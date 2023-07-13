//
//  useful_functions.hpp
//  CBM
//
//

#ifndef useful_functions_h
#define useful_functions_h

#include "ElementPool.hpp"
#include "types.hpp"
#include "birdID.hpp"
#include <nlohmann/json.hpp>
#include <map>
#include <sstream>
#include <string>
#include <iostream>
#include <sys/stat.h>


template <typename T>
std::string stringify_vec( const std::vector<T>& v, const std::string& sep = "," ) {
    std::stringstream ss;
    size_t size = v.size();
    for ( size_t i = 0; i < size; ++i ) {
        ss << v[i];
        if (i != size - 1)
            ss << sep;
    }
   return ss.str();
}

template <typename T>
std::string stringify_vec( const ElementPool<T>& v, const std::string& sep = "," ) {
    std::stringstream ss;
    int size = v.size();
    for ( uint i = 0; i < size; ++i ) {
        ss << v[i];
        if (i != size - 1)
            ss << sep;
    }
   return ss.str();
}

template <typename Enumeration>
auto as_integer(Enumeration const value)-> typename std::underlying_type<Enumeration>::type
{
    return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}

bool does_path_exist( const std::string &s );


namespace utils {
using std::to_string ;
std::string to_string( const BirdID& x ) ;
}



#endif /* useful_functions_h */
