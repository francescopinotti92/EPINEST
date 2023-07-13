//
//  infected_set.cpp
//  CBM
//
//

#include <stdio.h>

#include "infected_set.hpp"


//====== SparseEl ======//

// constructor (with pointer, fill element)
SparseEl::SparseEl(pBird* pt) {
    state.BirdPt = pt;
}

// constructor (with integer, free element)
SparseEl::SparseEl(uint next_) {
    state.next = next_;
}

// default constructor
SparseEl::SparseEl() {
    state.next = UINT_MAX;
}

//=== Setters and getters ===//

// get next free element
// must be used on free elements
uint SparseEl::getNext() const {
    return state.next;
}

// get pointer to dense
// must be used on fill elements
pBird* & SparseEl::getPt() {
    return state.BirdPt;
}


// morphs fill element into free element
void SparseEl::setNext(const uint& next_) {
    state.next = next_;
}

// morphs free element into fill element
void SparseEl::setPt(pBird* const& pt) {
    state.BirdPt = pt;
}

//====== infected_set =======//

// constructor
infected_set::infected_set(uint maxCapacity_): maxCapacity(maxCapacity_) {
   dense.reserve(maxCapacity);
   sparse.reserve(maxCapacity);
   mSize = 0;
   firstSusc = 0;
   
   firstAvailable = 0;
   
   // add null elements
   for (uint i = 0; i < maxCapacity; ++i) {
       dense.push_back(nullptr);
       sparse.emplace_back(); // create a free element
   }
   
   for (uint i = 0; i < maxCapacity - 1; ++i)
       sparse[i].setNext(i + 1); // create link to next free element
   sparse[maxCapacity - 1].setNext(UINT_MAX); //should not be necessary but whatever
   
}


// checks if bird is infectious and uses appropriate inserter function
// returns reference to inserted bird
Bird& infected_set::insert( pBird&& pBird_ ) {
   if ( pBird_->isInfectious() )
       return insertInfected( std::move( pBird_ ) );
   else
       return insertSusceptible( std::move( pBird_ ) );
}

// inserts a susceptible bird
// returns reference to inserted bird
Bird& infected_set::insertSusceptible( pBird&& pBird_ ) {
    
    assert( firstAvailable != UINT_MAX );

    uint index = firstAvailable;
    firstAvailable = sparse[firstAvailable].getNext();
    
    pBird_->index = index;

    dense[mSize] = std::move( pBird_ );
    sparse[index].setPt(&dense[mSize]);
    
    mSize++;
    
    return *dense[mSize - 1];
}

// inserts an infected bird
// returns reference to inserted bird
Bird& infected_set::insertInfected( pBird&& pBird_ ) {
    
    assert( firstAvailable != UINT_MAX );

    uint index = firstAvailable;
    firstAvailable = sparse[firstAvailable].getNext();
    
    pBird_->index = index; // bird must know which position it occupies in infectedVector
    dense[mSize] = std::move(pBird_);
    sparse[index].setPt(&dense[mSize]);

    std::swap( dense[mSize], dense[firstSusc] ); // a swap is needed to keep in order
    std::swap( sparse[dense[mSize]->index], sparse[dense[firstSusc]->index] );
    
    mSize++;
    firstSusc++;
    
    return *dense[firstSusc - 1];
}

// remove susceptible bird (by pointer)

// n.b. this function is crazy because it assumes pBird_ is a valid reference to a bird
// then calling reset on it is fine.

pBird&& infected_set::removeSusceptible( pBird&& pBird_ ) {
    uint index = pBird_->index;
    uint index2 = dense[mSize - 1]->index;
    
    ( *(sparse[index].getPt() ) ).reset(); // now *(sparse[index]) has been released
    std::swap( *(sparse[index].getPt() ),  *(sparse[index2].getPt() ));
    
    sparse[index2].setPt(&dense[index]);
    sparse[index].setNext(firstAvailable);
    firstAvailable = index;
    
    mSize--;
    
    return std::move(pBird_);
}

// remove infectious bird (by pointer)
pBird&& infected_set::removeInfected( pBird&& pBird_ ) {

    uint index = pBird_->index;
    uint index2 = dense[firstSusc - 1]->index;
    uint index3 = dense[mSize - 1]->index;

    ( *(sparse[index].getPt() ) ).reset(); // now *(sparse[index]) has been released
    
    std::swap( *(sparse[index].getPt() ), *(sparse[index2].getPt() ) ); // first swap with last infected bird
    std::swap( sparse[index], sparse[index2] );
    
    if ( index2 != index3 ) { // no second swap if i2 == i3
        std::swap( *(sparse[index].getPt() ), *(sparse[index3].getPt() ) ); // first swap with last infected bird
        std::swap( sparse[index], sparse[index3] );
    }
    
    sparse[index].setNext(firstAvailable);
    firstAvailable = index; // index is now free
    
    mSize--;
    firstSusc--;
    
    return std::move(pBird_);
}

// remove bird (by pointer)
pBird infected_set::remove(pBird&& pBird_) {

    if (pBird_->isInfectious()){
        return removeInfected(std::move(pBird_));
    }
    else {
        return removeSusceptible(std::move(pBird_));
    }
}

// remove susceptible bird (by index)
pBird infected_set::removeSusceptible(const uint& index) {
    
    uint index2 = dense[mSize - 1]->index;

    pBird birdOut = std::move(*(sparse[index].getPt() ) );
    
    std::swap( sparse[index].getPt(), sparse[index2].getPt() );         // swap ptrs
    std::swap( *(sparse[index].getPt()), *(sparse[index2].getPt() ) );  // swap ptrs content
    
    sparse[index].setNext(firstAvailable);
    
    firstAvailable = index;
    mSize--;
   
    return birdOut;
}

// remove infectious bird (by index)
pBird infected_set::removeInfected(const uint& index) {
    
    uint index2 = dense[firstSusc - 1]->index;
    uint index3 = dense[mSize - 1]->index;
    pBird birdOut = std::move( *( sparse[index].getPt() ) ); // get ownership of Bird
    
    std::swap( sparse[index].getPt(), sparse[index2].getPt() );
    std::swap( *(sparse[index].getPt()), *(sparse[index2].getPt()) );
    
    if ( index2 != index3 ) { // no second swap if i2 == i3
        std::swap( sparse[index].getPt(), sparse[index3].getPt() );
        std::swap( *(sparse[index].getPt()), *(sparse[index3].getPt() ) );
    }
    sparse[index].setNext(firstAvailable);
    firstAvailable = index;
    
    mSize--;
    firstSusc--;
          
    return birdOut;
}

// remove bird (by index)
// usually called by structures that hold indexes, e.g. vendors and crops
pBird infected_set::remove( const uint& index ) {
    if ( ( *(sparse[index].getPt() ) )->isInfectious() ) {
        return removeInfected( index );
    }
    else {
        return removeSusceptible( index );
    }
}

// removes last element from dense(useful if you need to move all birds at once)
pBird infected_set::removeLast() {
    /*
    pBird bird = std::move(*(sparse[mSize - 1].getPt() ) ); // get ownership of Bird
    mSize--;
    if (mSize < firstSusc)
        firstSusc = mSize;
     */
    
    uint indexLast = dense[mSize - 1]->index;
    pBird bird = remove( indexLast );
    
    return bird;
}

// remove the last element in dense
void infected_set::pop_back() {

    if (mSize > 0) {
        uint index = dense[mSize - 1]->index;
        dense[mSize - 1].reset();
        sparse[index].setNext(firstAvailable);
        firstAvailable = index;
        mSize--;
        if (mSize < firstSusc)
            firstSusc = mSize;
    }
    
}

// move a not-anymore-infectious bird close to other non-infectious birds
// to be called right after a bird becomes non-infectious
// Note: INVALIDATES any reference to bird at 'index'
void infected_set::swapToSusceptible(const uint& index) {
    
    uint index2 = dense[firstSusc - 1]->index;

    std::swap( sparse[index].getPt(), sparse[index2].getPt() );         // swap sparse content
    std::swap( *(sparse[index].getPt() ), *(sparse[index2].getPt() ) ); // swap dense content
    
    firstSusc--;
}

// move a not-anymore-susceptible bird close to other infectious birds
// to be called right after a bird becomes infectious
// Note: INVALIDATES any reference to bird at 'index'
void infected_set::swapToInfected(const uint& index) {
        
    uint index2 = dense[firstSusc]->index;

    std::swap( sparse[index].getPt(), sparse[index2].getPt() );         //swap sparse content
    std::swap( *(sparse[index].getPt() ), *(sparse[index2].getPt() ) ); // swap dense content
        
    firstSusc++;
}

//====== Access functions ======//

SparseEl& infected_set::operator[](const uint& sparseIndex) { return sparse[sparseIndex]; }
pBird& infected_set::getBirdDense(const uint& denseIndex)   { return dense[denseIndex]; }
pBird& infected_set::getBirdSparse(const uint& sparseIndex) { return *( sparse[sparseIndex].getPt() ); }

//====== Return various iterators ======//

pBirdVecType::iterator infected_set::begin() {
    return dense.begin();
}

pBirdVecType::const_iterator infected_set::cbegin() const {
    return dense.cbegin();
}

pBirdVecType::iterator infected_set::end() {
    return dense.begin() + mSize;
}
   
pBirdVecType::const_iterator infected_set::cend() const {
   return dense.cbegin() + mSize;
}

pBirdVecType::iterator infected_set::endInf() {
    return dense.begin() + firstSusc;
}

pBirdVecType::const_iterator infected_set::cendInf() const {
    return dense.cbegin() + firstSusc;
}

//====== Output functions ======//

void infected_set::printContent() {
    // prints index of each bird in dense
    for (auto it = begin(); it != end(); ++it)
        std::cout << (*it)->index << " ";
    std::cout << std::endl;
}

void infected_set::printStatus() {
    for (auto it = begin(); it != end(); ++it)
        std::cout << (*it)->isInfectious() << " ";
    std::cout << std::endl;
}

void infected_set::printRefCount() {
    for (auto it = begin(); it != end(); ++it)
        std::cout << it->use_count() << " ";
    std::cout << std::endl;
}

void infected_set::printRefCount(uint index_dense) {
    std::cout << dense[index_dense].use_count() << std::endl;
}

//====== Miscellanea ======//

void infected_set::clear( ChickenPool* birdMngr ) {
    while( !empty() ) {
        pBird bird = removeLast();
        birdMngr->return_bird( std::move( bird ) );
    }
}

// returns number of birds in container
uint infected_set::size() {
    return mSize;
}

// returns number of infected birds in container
uint infected_set::size_infected() {
    return firstSusc;
}

// returns current capacity of container
uint infected_set::capacity() {
    return maxCapacity;
}

uint infected_set::available_space() {
    return maxCapacity - mSize;
}

bool infected_set::empty() {
    return (mSize == 0) ? true : false;
}

// returns first available index in container
// next bird to be inserted in container will be assigned firstAvailable as index
uint infected_set::getFirstAvailableIndex() {return firstAvailable;}


//====== Tests ======//

// checks if bird index matches content in sparse set
void infected_set::check_index() {
    for (uint i = 0; i < mSize; i++) {
        uint idx1 = dense[i]->index;
        uint idx2 = (*(sparse[idx1].getPt()))->index;
        assert(idx1 == idx2);
    }
}

void infected_set::print_index() {
    std::cout << "birds: " << mSize << std::endl;
    for (uint i = 0; i < mSize; i++) {
        uint idx1 = dense[i]->index;
        uint idx2 = (*(sparse[idx1].getPt()))->index;
        std::cout << idx1 << " " << idx2;
        std::cout << (idx1 != idx2 ? " error" : "") << std::endl;
    }
}

// checks that infected birds (if any) are always before susceptible ones in dense
void infected_set::check_infected_adjacent() {
    bool found_susceptible = false;
    bool infected_after_susceptible = false;
    for (uint i = 0; i < mSize; i++) {
        bool isInf = dense[i]->isInfectious();

        if (!isInf)
            found_susceptible = true;
        
        if (isInf and found_susceptible)
            infected_after_susceptible = true;
        
        assert( !(found_susceptible and infected_after_susceptible) );
    }
}

// checks ref count of birds in container is 1
// N.B. (1)

void infected_set::check_refcount() {
    for (uint i = 0; i < mSize; ++i)
    assert( dense[i].use_count() == 1 );
}
