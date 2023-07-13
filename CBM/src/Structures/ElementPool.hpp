//
//  ElementPool.hpp
//  CBM
//
//

#ifndef ElementPool_h
#define ElementPool_h

#include <vector>
#include <cassert>
#include <stdint.h>

typedef uint32_t uint;

// simple container that stores a set of elements
// Properties:
// 1) Performs only assignments and swaps (improve over push_back)
// 2) Remove from back for fast removals
// 3) Do not add elements beyond pool's capacity (may cause seg fault or undefined behavior)
template <typename T>
class ElementPool {
public:
    ElementPool( const unsigned int& capacity ) {
        assert( capacity > 0 );
        _capacity = capacity;
        v = std::vector<T>( capacity, T() );
        lastElem = -1;
    }
    
    void insert( const T& elem ) {
        ++lastElem;
        v[lastElem] = elem;
    }
    
    void insert( const T&& elem ) {
        ++lastElem;
        v[lastElem] = std::move( elem );
    }
    
    void remove( const T& elem ) {
        int currElem = 0;
        while( currElem <= lastElem  ) {
            if ( v[currElem] == elem ) {
                v[currElem] = v[lastElem];
                --lastElem;
                break;
            }
            else
                ++currElem;
        }
    }
    
    void increaseCapacity( const int& capacity ) {
        if ( _capacity >= capacity )
            return;
        v.resize( capacity );
        _capacity = capacity;
    }
    
    void addCapacity( const int& amount ) {
        
        v.resize( _capacity + amount );
        _capacity += amount;
        
    }
   
    T& peekLast() {
        return v[lastElem];
    }
    
    T removeLast() {
        T res = v[lastElem];
        --lastElem;
        return res;
    }
    
    T& operator[]( const uint& pos ) {
        return v[pos];
    }
    
    const T& operator[]( const uint& pos ) const {
        return v[pos];
    }
    
    void clear() { lastElem = -1; };
    bool empty() { return ( lastElem >= 0 ) ? false : true; }
    bool isFull() { return ( lastElem + 1 == _capacity ) ? true : false; }
    
    unsigned int size() const { return lastElem + 1; }
    unsigned int capacity() const { return _capacity; }
    
    typename std::vector<T>::iterator begin() { return v.begin(); }
    typename std::vector<T>::iterator end()   { return v.begin() + lastElem + 1; }

private:
    std::vector<T> v;
    int lastElem;
    unsigned int _capacity;
};

#endif /* ElementPool_h */
