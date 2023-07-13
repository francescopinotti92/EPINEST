//
//  checks.hpp
//  CBM
//
//

#ifndef checks_h
#define checks_h

#include "warnings.hpp"

// some functions to check numerical conditions

template <typename T>
void checkEquality( const T& x, const T& y ) {
    if ( x != y )
        throw Warning( "x, y are different." );
}

template <typename T>
void checkSmaller( const T& x, const T& y ) {
    if ( x > y )
        throw Warning( "x is larger than y" );
}

template <typename T>
void checkStrictlySmaller( const T& x, const T& y ) {
    if ( x >= y )
        throw Warning( "x is larger or equal than y" );
}

template <typename T>
void checkLarger( const T& x, const T& y ) {
    if ( x < y )
        throw Warning( "x is smaller than y" );
}

template <typename T>
void checkStrictlyLarger( const T& x, const T& y ) {
    if ( x <= y )
        throw Warning( "x is smaller or equal than y" );
}

template <typename T>
void checkPositive( const T& val ) {
    if ( val < 0 )
        throw Warning( "Passed value is negative." );
}

template <typename T>
void checkStrictlyPositive( const T& val ) {
    if ( val <= 0 )
        throw Warning( "Passed value is not strictly positive." );
}

template <typename T>
void checkUnitInterval( const T& val ) {
    if ( ( val < 0. ) or ( val > 1. ) )
        throw Warning( "Passed value should be between 0 and 1." );
}

// some checks for json

void requireJsonKey( const nlohmann::json& js, const std::string& key );

#endif /* checks_h */
