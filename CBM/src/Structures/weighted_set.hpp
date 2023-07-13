//
//  weighted_set.hpp
//  CBM
//
//

#ifndef weighted_set_hpp
#define weighted_set_hpp

#include <stdio.h>

#include "random.hpp"
#include "types.hpp"
#include <vector>
#include <algorithm>


template <typename T>
struct weightedElem {
    weightedElem( T* elem_, double weight_): elem( elem_ ), weight( weight_ ) {};
    T* elem;
    double weight;
};

template <typename T>
class weighted_set {
public:
    weighted_set() {
        elems = {};
    }
    void addNbr( T* newElem, double weight ) {
        elems.emplace_back( newElem, weight );
    }
    weightedElem<T>& operator[]( const uint& index ) {
        return elems[index];
    }
    uint size() {
        return static_cast<uint>( elems.size() );
    }
    typename std::vector<weightedElem<T>>::const_iterator cbegin() { return elems.cbegin(); }
    typename std::vector<weightedElem<T>>::const_iterator cend() { return elems.cend(); }
private:
    std::vector<weightedElem<T>> elems;
};

#endif /* weighted_set_hpp */
