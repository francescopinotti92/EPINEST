//
//  infected_set.hpp
//  CBM_ECS
//
//

#ifndef infected_set_h
#define infected_set_h

#include <vector>
#include <queue>
#include <algorithm>
#include <memory>

#include "ChickenPool.hpp"
#include "types.hpp"
#include "bird.hpp"



//====== SparseEl (Sparse Element) ======//

// SparseEl wraps either a pointer to a bird or an integer marking position to next free spot
// in infected set.

struct SparseEl {
public:
    SparseEl(pBird* pt);
    SparseEl(uint next_);
    SparseEl();
    uint getNext() const;
    void setNext(const uint& next_);
    pBird* & getPt();
    void setPt(pBird* const& pt);
private:
    union {
        pBird* BirdPt;
        uint next;
    } state;
};

typedef std::vector<pBird> pBirdVecType;
typedef std::vector<SparseEl> ppBirdVecType;


//====== infected_set ======//

/*
 DESCRIPTION
 
 'infected_set' is a core data structure in this project and is responsible for storing birds.
 It is organized as a sorted sparse vector holding std::shared_ptr<Bird> (pBird) objects: each instance
 includes a 'sparse' and a 'dense' vectors with the same, constant capacity. pBird objects
 are stored contiguously in the dense layer; each pBird is also assigned an entry in the sparse layer,
 which simply contains a pointer to itself. Occupied elements in the sparse layer are not arranged
 contiguosly, however each pBird instance knows its own index in the sparse layer,
 which can be accessed through the 'index' member variable.
 
 A crucial aspect of 'infected_set' is that items in the dense layer are sorted according to whether
 birds are infectious or not: infectious birds are stored contiguously right at the beginning of the
 layer, while non-infectious birds are stored contiguously immediately after the infectious ones.
 The infectious state of a bird can be checked through the 'isInfectious()' method.
 
 A GRAPHICAL EXAMPLE
 
 The following scheme illustrates an 'infected_set' capacity 6 holding
 2 infectious birds (I), 2 non-infectious birds (NI) and 2 empty buckets.
 
 'pBird' instances are stored contiguosly in the dense layer (D), sorted according
 to their infectious state.
 
 Each occupied element (i1,i2,...) in the sparse layer (S) points to a unique element in D.
 
 *--- I ---*-- NI ---*- empty -*
 |----|----|----|----|----|----|
 | p1 | p2 | p3 | p4 | xx | xx |  D
 |----|----|----|----|----|----|
   +      +    +    +
   |       \    \    \_____
   |        \    \         \
 |----|----|----|----|----|----|
 | i1 | xx | i2 | i3 | xx | i4 |  S
 |----|----|----|----|----|----|
 
 
 KEEPING THE DENSE LAYER SORTED AND COMPACT
 
 if a bird becomes I or NI, both D & S layers must be updated.
 
 -  If a bird becomes I from NI, it is swapped with the leftmost NI bird.
    for example, if p4 becomes I, it must be swapped with p3. Please note
    that the corresponding pointers in S must be swapped.
 
 *--- I ---*-- NI ---*- empty -*          *----- I ------*-NI-*- empty -*
 |----|----|----|----|----|----|          |----|----|----|----|----|----|
 | p1 | p2 | p3 | p4 | xx | xx |          | p1 | p2 | p4 | p3 | xx | xx |
 |----|----|----|----|----|----|          |----|----|----|----|----|----|
   +      +    +    +                       +      +   +   +
   |       \    \    \_____      ........   |       \   \_ | __
   |        \    \         \                |        \     |   \
 |----|----|----|----|----|----|          |----|----|----|----|----|----|
 | i1 | xx | i2 | i3 | xx | i4 |          | i1 | xx | i2 | i3 | xx | i4 |
 |----|----|----|----|----|----|          |----|----|----|----|----|----|
 
 
 -  If a bird becomes NI from I, it is swapped with the rightmost I bird.
    for example, if p1 becomes NI, it must be swapped with p2. Please note
    that the corresponding pointers in S must be swapped.
 
 *--- I ---*-- NI ---*- empty -*          *-I -*----- NI -----*- empty -*
 |----|----|----|----|----|----|          |----|----|----|----|----|----|
 | p1 | p2 | p3 | p4 | xx | xx |          | p2 | p1 | p3 | p4 | xx | xx |
 |----|----|----|----|----|----|          |----|----|----|----|----|----|
   +      +    +    +                      +    +       +   +
   |       \    \    \_____      ........   \_ /_____    \   \______
   |        \    \         \                  /      \    \         \
 |----|----|----|----|----|----|          |----|----|----|----|----|----|
 | i1 | xx | i2 | i3 | xx | i4 |          | i1 | xx | i2 | i3 | xx | i4 |
 |----|----|----|----|----|----|          |----|----|----|----|----|----|
 
 
 ADDING BIRDS
 
 Adding a new bird (p5) requires first a free slot in S.
 
 Unoccupied elements (e1,e2 in this example) in S form a linked-list that is used as a free list,
 i.e. a pool of re-usable items. In this example, e1 is the head of the list, while e2 is the next
 (and last) element.
 
 When a new bird is added, we pop the head (H) of the free list and assign the corresponding slot in L
 
 *--- I ---*-- NI ---*- empty -*
 |----|----|----|----|----|----|
 | p1 | p2 | p3 | p4 | xx | xx |  D
 |----|----|----|----|----|----|
   +      +    +    +
   |       \    \    \_____             Free list: e1 --+ e2
   |        \    \         \
 |----|----|----|----|----|----|
 | i1 | e2 | i2 | i3 | e1 | i4 |  S
 |----|----|----|----|----|----|
         +            | +
         |____________| |
                         \
                          H (use this slot)

 
 Next, we assign a slot in D. This step differs to whether a bird is I or NI.
 
 - If bird is NI, simply place p5 in the leftmost empty slot in D.
 
 *--- I ---*-- NI ---*- empty -*          *--- I ---*----- NI -----*-em-*
 |----|----|----|----|----|----|          |----|----|----|----|----|----|
 | p1 | p2 | p3 | p4 | xx | xx |          | p1 | p2 | p3 | p4 | p5 | xx |
 |----|----|----|----|----|----|          |----|----|----|----|----|----|
   +      +    +    +                       +      +    +   +    +
   |       \    \    \_____      ........   |       \    \   \___|__
   |        \    \         \                |        \    \      |  \
 |----|----|----|----|----|----|          |----|----|----|----|----|----|
 | i1 | e2 | i2 | i3 | e1 | i4 |          | i1 | e2 | i2 | i3 | i5 | i4 |
 |----|----|----|----|----|----|          |----|----|----|----|----|----|
         +            | +                         +
         |____________| |                         |
                        H                         H
 
 -  If bird is I, first place p5 in the leftmost empty slot in D and
    then swap it with the leftmost NI bird
  
 *--- I ---*-- NI ---*- empty -*          *----- I ------*-- NI ---*-em-*
 |----|----|----|----|----|----|          |----|----|----|----|----|----|
 | p1 | p2 | p3 | p4 | xx | xx |          | p1 | p2 | p5 | p4 | p3 | xx |
 |----|----|----|----|----|----|          |----|----|----|----|----|----|
   +      +    +    +                       +      +    +    +_+
   |       \    \    \_____      ........   |       \    \____/\____
   |        \    \         \                |        \       /\     \
 |----|----|----|----|----|----|          |----|----|----|----|----|----|
 | i1 | e2 | i2 | i3 | e1 | i4 |          | i1 | e2 | i2 | i3 | i5 | i4 |
 |----|----|----|----|----|----|          |----|----|----|----|----|----|
         +            | +                         +
         |____________| |                         |
                        H                         H

 REMOVING BIRDS
 
 Removing birds is similar to adding them, but operations must be done in reverse order.
 Again, removing from D requires different steps according to whether a bird is I or NI
 
 -  If bird is NI: swap it with the rightmost NI bird. Now pop the last element in D and
    return the corresponding sparse slot to the free list (it becomes the new head H).
 
 -  If bird is I: first swap it with the rightmost I bird and then with the rightmost NI bird.
    Now pop the last element in D and return the corresponding sparse slot to the free list
    (it becomes the new head H).
 
 NOTES:
 
 1) Exceeding capacity by adding too many birds results in an error
 2) Bird transfers from one 'infected_set' to another should update a birds's index accordingly
 3) Functions modifying a bird's infectious status must call 'swapToSusceptible' or
    'swapToInfected' methods accordingly to keep the dense layer sorted. Example: In a SEIR model,
    birds transitioning from S to E do not require a swap operation since E birds are not infectious.
 
 
 */

class infected_set {
public:
     
    infected_set(uint maxCapacity_ = 0);
        
    Bird& insertSusceptible(pBird&& pBird_);
    Bird& insertInfected(pBird&& pBird_);
    Bird& insert(pBird&& pBird_);

    pBird&& removeSusceptible(pBird&& pBird_);
    pBird&& removeInfected(pBird&& pBird_);   
    pBird remove(pBird&& pBird_);
    
    pBird removeSusceptible(const uint& index);
    pBird removeInfected(const uint& index);
    pBird remove(const uint& index);
    
    pBird removeLast();
    void pop_back();
    
    void swapToSusceptible(const uint& index);
    void swapToInfected(const uint& index);
    
    /*
    Access functions
    */
    
    SparseEl& operator[](const uint& sparseIndex);
    pBird& getBirdDense(const uint& denseIndex);
    pBird& getBirdSparse(const uint& sparseIndex);
    
    /*
    Return various iterators
    */
    
    pBirdVecType::iterator begin();
    pBirdVecType::const_iterator cbegin() const;
    pBirdVecType::iterator end();
    pBirdVecType::const_iterator cend() const;
    pBirdVecType::iterator endInf();
    pBirdVecType::const_iterator cendInf() const;
    
    /*
     Output functions
    */
    
    void printContent();
    void printStatus();
    void printRefCount();
    void printRefCount(uint index_dense);
    
    /*
     Miscellanea
    */
    
    void clear( ChickenPool* birdMngr );
    uint size();
    uint size_infected();
    uint capacity();
    uint available_space();
    bool empty();
    uint getFirstAvailableIndex();

    /*
     Tests
     */
    
    void check_index();
    void print_index();
    void check_infected_adjacent();
    void check_refcount();
    
private:
    pBirdVecType dense;
    ppBirdVecType sparse;

    uint maxCapacity;
    uint mSize; // number of elements in dense
    uint firstSusc; // dense index of first susceptible element
    uint firstAvailable; // sparse index first free element
    
};



#endif /* infected_set_h */
