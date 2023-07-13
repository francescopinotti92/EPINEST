//
//  sparse_set.hpp
//  CBM_ECS
//
//

#ifndef sparse_set_h
#define sparse_set_h

#include <vector>
#include <memory>

typedef unsigned int uint;

template <typename T>
struct sparse_set_el {
    sparse_set_el(T el) {state.elem = el;}
    sparse_set_el() {state.next = UINT_MAX;}
    uint getNext() const {return state.next;}
    void setNext(const uint& next_) {state.next = next_;}
    T& getEl() {return state.BirdPt;}
    void setEl(T el) {state.BirdPt = el;}
private:
    union {
        T elem;
        uint next;
    } state;
};

template <typename T>
class sparse_set {
    typedef std::shared_ptr<T> Pt;

    std::vector<Pt> dense;
    std::vector< sparse_set_el<Pt> > sparse;

    uint maxCapacity;
    uint mSize; // number of elements in dense
    uint firstAvailable; // sparse index first free element
    
public:
    sparse_set(uint maxCapacity_ = 0): maxCapacity(maxCapacity_) {
        dense.reserve(maxCapacity);
        sparse.reserve(maxCapacity);
        mSize = 0;
        firstAvailable = 0;
        
        // add elements
        for (uint i = 0; i < maxCapacity; ++i) {
            dense.push_back(nullptr);
            sparse.emplace_back(); // create a free element
        }
        
        for (uint i = 0; i < maxCapacity - 1; ++i)
            sparse[i].setNext(i + 1); // create link to next free element
        sparse[maxCapacity - 1].setNext(UINT_MAX); //should not be necessary but whatever
    };
    
    T& insert(Pt&& newEl) {
        
        assert( firstAvailable != UINT_MAX );
        
        uint index = firstAvailable;
        firstAvailable = sparse[firstAvailable].getNext();
        
        newEl->index = index;

        dense[mSize] = std::move(newEl);
        sparse[index].setPt(&dense[mSize]);
        
        mSize++;
        
        return *dense[mSize - 1];
    }
    
    // remove element and move it outside the sparse_set
    Pt&& remove(Pt&& elem) {
        uint index2 = dense[mSize - 1]->index;
        uint index = elem->index;
        
        ( *(sparse[index].getPt() ) ).reset(); // now *(sparse[index]) has been released
        ( *(sparse[index].getPt() ) ).swap( *(sparse[index2].getPt() ) );
        sparse[index2].setPt(&dense[index]);
        sparse[index].setNext(firstAvailable);
        firstAvailable = index;
        
        mSize--;

        return std::move(elem);
    }
    
    // remove element and move it outside the sparse set (by index)
    Pt&& remove(const uint& index) {

        uint index2 = dense[mSize - 1]->index;
        Pt&& tmp = std::move(*(sparse[index].getPt() ) );

        ( *(sparse[index].getPt() ) ).swap( *(sparse[index2].getPt() ) );
        sparse[index2].setPt(&dense[index]);
       
        sparse[index].setNext(firstAvailable);
        firstAvailable = index;
        mSize--;
       
        return std::move(tmp);
    }
    
};

#endif /* sparse_set_h */
