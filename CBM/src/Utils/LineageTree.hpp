//
//  LineageTree.hpp
//  CBM
//
//

#ifndef LineageTree_hpp
#define LineageTree_hpp

#include "useful_functions.hpp"
#include "types.hpp"
#include "birdID.hpp"
#include <iostream>
#include <stdio.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

//====== DataLineage======//

// Holds metadata about individual infection events
struct DataLineage {
    
    DataLineage( const std::string& loc = "" ): loc( loc ) {} ;
    std::string loc ;
    
} ;

namespace utils {
using std::to_string ;
std::string to_string( const DataLineage& data ) ;
} // ?? Maybe remove this

// contains data about sampling
// i) Sampling time, ii) Sampling location
struct DataLineageSampling {
    
    DataLineageSampling(): tSample( 0 ), locSample( "NA" ) {} ;
    DataLineageSampling( const uint& tSample, const std::string& locSample ): tSample( tSample ), locSample( locSample ) {} ;
    uint tSample ;
    std::string locSample ;
    
} ;


std::string convertData2NHX( const DataLineage& data ) ;




//====== LineageTreeNode ======//

template <typename T, typename U>
struct LineageTreeNode {
    LineageTreeNode( const T& lng, const U& data, const uint& t, const bool& extant, LineageTreeNode<T,U>* parent = nullptr  ) ;
    void eraseChild( LineageTreeNode<T,U>* child ) ;
    uint getSizeChildren();
    uint t ; // birth time
    uint tSample ; // sampling time
    std::string locSample ; // sampling location
    T lng ;
    U data ;
    LineageTreeNode<T,U>* parent ;
    std::vector<LineageTreeNode<T,U>*> children ;
    bool extant ; // true if still around in simulation
    bool needed ; // true if required in reduced transmission tree
    bool sampled ; // true if sampled
    
} ;

/*
 Deletes nodes recursively starting from a root node.
 The root node is deleted only after each of its children
 has been deleted.
 */
template<typename T, typename U>
void deleteLineageTreeNodeTree( LineageTreeNode<T,U>* root ) {
    
    while( not root->children.empty()  ) {
        
        deleteLineageTreeNodeTree( root->children.back() ) ;
        root->children.pop_back() ;
        
    }
    
    delete root ;
    
}

//====== LineageTree ======//

template <typename T, typename U, class Hash = std::hash<T>>
class LineageTree {
public:
    LineageTree() ;
    void reset() ;
    void addExtantLineage( const uint& t, const T& lng, const U& data, const T& lngParent ) ;
    void addExtantLineageExternal( const uint& t, const T& lng, const U& data ) ;
    void removeExtantLineage( const T& lng ) ;
    void sampleExtantLineage( const T& lng, const uint& t, const std::string& locSample ) ;
    std::vector<T> getSampledLineages( LineageTreeNode<T,U>* rootNode ) ;

    //std::vector<LineageTreeNode<T,U>*> subSampleTree( const uint& t, const std::vector<T>& sampledLngs ) ;
    std::vector<LineageTreeNode<T,U>*> subSampleTree( const std::unordered_map<T,DataLineageSampling,Hash>& sampledLngsInfo ) ;
    std::vector<LineageTreeNode<T,U>*> subSampleTree() ; // makes use of already sampled lineages

    LineageTreeNode<T,U>* getRootNode( LineageTreeNode<T,U>* lngNode ) ;
    
    uint getSizeExtantLineages() ;
    uint getSizeNodes() ;
    void printEdges() ;
    void printExtantLineages();
    
private:
    std::unordered_map<T, LineageTreeNode<T,U>*, Hash > extantLngs;
    std::unordered_set<LineageTreeNode<T,U>*> roots;
    void notifyParent( LineageTreeNode<T,U>* parent, LineageTreeNode<T,U>* child ) ;
    void mergeParentChild( LineageTreeNode<T,U>* midNode ) ;
    
    LineageTreeNode<T,U>* extractSubTree( LineageTreeNode<T,U>* node, LineageTreeNode<T,U>* parent ) ;
    
    void getSampledLineagesRecursive( LineageTreeNode<T,U>* node, std::vector<T>& lngs ) ;

    bool markNodeNeeded( LineageTreeNode<T,U>* node, const std::vector<T>& sampledLngs ) ;

    void printParentChildEdge( LineageTreeNode<T,U>* child, bool recursive = true );

    uint nnodes ;
    
    
} ;

// helper function used to sort nodes according to some attributes (typically time)


/*
template <typename T>
bool nodeSorter( const T& node1, const T& node2 );
 //Orders LineageTreeNode instances according to lineage birth time

bool nodeSorter( LineageTreeNode<BirdID,DataLineage>* node1, LineageTreeNode<BirdID,DataLineage>* node2 ) {
    
    return node1->t < node2->t ;
    
}
 */



//====== PhyloNode ======//

/*
 Phylogenetic tree building block
 
 Holds info about:
 - internal nodes
 - leaf nodes
 - node metadata
 - node timing
 - branch length
 
*/

template <typename T, typename U>
struct PhyloNode {
    
    PhyloNode( const T& lng, PhyloNode* parent = nullptr  ): lng( lng ), leftChild( nullptr ), rightChild( nullptr ), parent( parent ), depth( 0 ), t( 0 ), dt( 0 ), locSample( "NA" ) {} ;
    PhyloNode* leftChild ;
    PhyloNode* rightChild ;
    PhyloNode* parent ;
    uint depth ; // initialised to 0, used to track depth of internal nodes
    uint t ; // node time (infection time if internal, sampling time if leaf)
    uint dt ; // branch length (wrt parent)
    std::string locSample ; // sampling location (if any; default is NA)
    T lng ;  // lineage identity
    U data ; // extra data
        
} ;

template <typename T, typename U>
void deletePhyloNodeTree( PhyloNode<T,U>* root ) {
    
    if ( root->leftChild != nullptr )
        deletePhyloNodeTree( root->leftChild ) ;
    
    if ( root->rightChild != nullptr )
        deletePhyloNodeTree( root->rightChild ) ;
    
    delete root ;
    
}

/*
 Returns phylogenetic tree from reduced transmission tree.
 Sampled lineages appear as leaf nodes.
 This function specialization must be applied to a ROOT node
 */
template <typename T, typename U>
PhyloNode<T,U>* getAncestralTree( LineageTreeNode<T,U>* node ) {
    
    if ( node == nullptr )
        return nullptr ;
    
    bool isSampled = node->sampled ;
    
    PhyloNode<T,U>* newNode = new PhyloNode<T,U>( node->lng, nullptr ) ;
    
    // get depth
    newNode->depth = 0 ;
    
    auto& children = node->children ;
    uint nChildren = static_cast<uint>( children.size() ) ;
    
    bool isLeaf = ( newNode->depth == nChildren ) ? true : false ;
    
    if ( isLeaf ) {
        
        // create a leaf node
        assert( node->sampled ) ;
        newNode->t = node->tSample ;
        newNode->dt = 0. ;
    
        // add data
        newNode->data = node->data ;
        newNode->locSample = node->locSample ;
        
    }
    else { // process children
        
        assert( nChildren > 0 ) ;
        //std::sort( children.begin(), children.end(), nodeSorter<LineageTreeNode<BirdID,DataLineage>*> ) ;
        
        auto sorter = []( LineageTreeNode<BirdID,DataLineage>* node1, LineageTreeNode<BirdID,DataLineage>* node2 ) -> bool { return node1->t < node2->t ; } ;
        std::sort( children.begin(), children.end(), sorter ) ;

        
        newNode->t = children[ newNode->depth ]->t ;
        newNode->dt = 0. ;
        // add data
        newNode->data = node->data ;

                
        if ( isSampled ) { // node is sampled
            
            auto& child = children[ newNode->depth ] ;
            newNode->leftChild  = getAncestralTree( child, newNode ) ;
            newNode->rightChild = getAncestralTree( node,  newNode ) ;
            
        }
        else { // node is not sampled
            
            assert( nChildren > 1 ) ;
            if ( newNode->depth < nChildren - 2 ) { //
                
                auto& child = children[ newNode->depth ] ;
                newNode->leftChild  = getAncestralTree( child, newNode ) ;
                newNode->rightChild = getAncestralTree( node,  newNode ) ;
                
                
            }
            else {
                
                auto& child1 = children[ newNode->depth ] ;
                auto& child2 = children[ newNode->depth + 1 ] ;

                newNode->leftChild  = getAncestralTree( child1, newNode ) ;
                newNode->rightChild = getAncestralTree( child2, newNode ) ;
                
            }
            
        }
        
        
    }
    
    return newNode ;
}

/*
 Returns phylogenetic tree from reduced transmission tree.
 Sampled lineages appear as leaf nodes.
 This function specialization must be applied to NON-ROOT nodes.
 It is used recursively to build the tree.
 */
template <typename T, typename U>
PhyloNode<T,U>* getAncestralTree( LineageTreeNode<T,U>* node, PhyloNode<T,U>* phyloParent ) {
    
    if ( node == nullptr )
        return nullptr ;
    
    bool isSampled = node->sampled ;
    
    PhyloNode<T,U>* newNode = new PhyloNode<T,U>( node->lng, phyloParent ) ;
    
    if ( newNode->lng == phyloParent->lng )
        newNode->depth = phyloParent->depth + 1 ;
    else
        newNode->depth = 0 ;
   
    auto& children = node->children ;
    uint nChildren = static_cast<uint>( children.size() ) ;
    
    bool isLeaf = ( newNode->depth == nChildren ) ? true : false ;
    
    if ( isLeaf ) {
        // create a leaf node
        assert( node->sampled ) ;
        
        newNode->t = node->tSample ;
        newNode->dt = newNode->t - phyloParent->t ;
        
        assert( newNode->t >= phyloParent->t ) ;

        // add data
        newNode->data = node->data ;
        newNode->locSample = node->locSample ;
    }
    else { // process children
        
        assert( nChildren > 0 ) ;
        //std::sort( children.begin(), children.end(), nodeSorter<LineageTreeNode<BirdID,DataLineage>*> ) ;
        auto sorter = []( LineageTreeNode<BirdID,DataLineage>*& node1, LineageTreeNode<BirdID,DataLineage>* node2 ) -> bool { return node1->t < node2->t ; } ;
        std::sort( children.begin(), children.end(), sorter ) ;
        
        newNode->t = children[ newNode->depth ]->t ;
        newNode->dt = newNode->t - phyloParent->t ;
        
        assert( newNode->t >= phyloParent->t ) ;

        // add data
        newNode->data = node->data ;

                
        if ( isSampled ) { // node is sampled
            
            auto& child = children[ newNode->depth ] ;
            newNode->leftChild  = getAncestralTree( child, newNode ) ;
            newNode->rightChild = getAncestralTree( node,  newNode ) ;
            
        }
        else { // node is not sampled
            
            assert( nChildren > 1 ) ;
            if ( newNode->depth < nChildren - 2 ) { //
                
                auto& child = children[ newNode->depth ] ;
                newNode->leftChild  = getAncestralTree( child, newNode ) ;
                newNode->rightChild = getAncestralTree( node,  newNode ) ;
                
                
            }
            else {
                
                auto& child1 = children[ newNode->depth ] ;
                auto& child2 = children[ newNode->depth + 1 ] ;

                newNode->leftChild  = getAncestralTree( child1, newNode ) ;
                newNode->rightChild = getAncestralTree( child2, newNode ) ;
                
            }
            
        }
        
    }
    
    return newNode ;
    
}
/*
Yields a phylogenetic tree in Newick format
using the recursive function
*/
template <typename T, typename U>
std::string getNewick( PhyloNode<T,U>* root ) {
    
    std::string nwk ; // holds result
    PhyloNode2Newick( nwk, root ) ; // begin recursion
    nwk += ";" ; // closing character
    
    return nwk ;
    
}

/*
Recursive function to build a phylogenetic tree in Newick format
 */
template <typename T, typename U>
void PhyloNode2Newick( std::string& nwk, PhyloNode<T,U>* node ) {
    
    bool isLeaf = ( node->leftChild == nullptr ) ? true : false ;
    
    if ( isLeaf ) {
        
        nwk += utils::to_string( node->lng ) + ":" + std::to_string( node->dt ) ;
        
    }
    else { // manage branching event
        
        nwk += "(" ;
        PhyloNode2Newick( nwk, node->leftChild ) ;
        nwk += "," ;
        PhyloNode2Newick( nwk, node->rightChild ) ;
        nwk += ")" ;
        nwk += utils::to_string( node->lng ) ;
        nwk += ":" + std::to_string( node->dt ) ;

    }
    
    
}

/*
Yields a phylogenetic tree in NHX format
using the recursive function
*/
template <typename T, typename U>
std::string getNHX( PhyloNode<T,U>* root ) {
    
    std::string nhx ; // holds result
    PhyloNode2NHX( nhx, root ) ; // begin recursion
    nhx += ";" ; // closing character
    
    return nhx ;
    
}

/*
Recursive function to build a phylogenetic tree in NHX format
 */
template <typename T, typename U>
void PhyloNode2NHX( std::string& nhx, PhyloNode<T,U>* node ) {
    
    bool isLeaf = ( node->leftChild == nullptr ) ? true : false ;
    
    if ( isLeaf ) {
        
        nhx += utils::to_string( node->lng ) + ":" + std::to_string( node->dt ) ;
        nhx += fmt::format( "[&&NHX:{}:locSample={}:t={}]", convertData2NHX( node->data ),
                           node->locSample, node->t ) ;
        
    }
    else { // manage branching event
        
        nhx += "(" ;
        PhyloNode2NHX( nhx, node->leftChild ) ;
        nhx += "," ;
        PhyloNode2NHX( nhx, node->rightChild ) ;
        nhx += ")" ;
        nhx += fmt::format( "{}-{}", utils::to_string( node->lng ), node->depth ) ;
        //nhx += utils::to_string( node->lng ) ;
        nhx += ":" + std::to_string( node->dt ) ;
        nhx += fmt::format( "[&&NHX:{}:t={}]", convertData2NHX( node->data ), node->t ) ;

    }
    
}


/*
// assumes a trimmed tree
template <typename T, typename U, class Hash = std::hash<T>>
std::string getAncestralTreeNewick( LineageTreeNode<T,U>* root, const std::unordered_set<T,Hash>& sampledLngs ) {
    
    
    std::string nwk = "" ; // holds newick string
    
    buildNewickRecursive( nwk, root, sampledLngs ) ;
    
    nwk += ";" ; // close newick string
    
    return nwk ;
    
}




template <typename T, typename U, class Hash = std::hash<T>>
void buildNewickRecursive( std::string& nwk, LineageTreeNode<T,U>* node, const std::unordered_set<T, Hash>& sampledLngs ) {
    
    
    bool isSampled = sampledLngs.find( node->lng ) != sampledLngs.end() ;
    auto& children = node->children ;
    
    // order children in from earliest to latest
    //std::sort( children.begin(), children.end(), nodeSorter ) ;
    std::sort( children.begin(), children.end() ) ;
    uint nChildren = static_cast<uint>( children.size() ) ;
    
    if ( nChildren == 0 ) { // node is leaf (and is sampled by assumption)
        
        //nwk += std::to_string( node->lng ) ;
        nwk += utils::to_string( node->lng ) ;
        nwk += ":" + utils::to_string( node->data ) ;
        
    }
    else { // node is internal
        
        // all children except last one
        for ( uint i = 0 ; i < nChildren - 1 ; ++i ) {
            
            nwk += "(" ; // new transmission event induces a splitting
            buildNewickRecursive( nwk, children[i], sampledLngs ) ;
            nwk += "," ;
            
        }
        
        // last child
        if ( isSampled ) { // node is sampled
            
            nwk += "(";
            buildNewickRecursive( nwk, children.back(), sampledLngs ) ;
            //nwk += "," + std::to_string( node->lng );
            nwk += "," + utils::to_string( node->lng );
            nwk += ":" + utils::to_string( node->data ) ;


            nwk += ")" ;
            
        }
        else {
            
            buildNewickRecursive( nwk, children.back(), sampledLngs ) ;
            //nwk += ")" ;

        }
        for ( uint i = 0 ; i < nChildren - 1 ; ++i ) {
            nwk += ")" ;
        }

        
        //nwk += ")" ;

        
    }
    
}

*/






//====== Helper functions ======//


template <typename T, typename U>
LineageTreeNode<T,U>* eliminateRedundantNodes( LineageTreeNode<T,U>* root, const std::vector<T>& sampledLngs ) ;

// find root starting from 'lngNode'
template <typename T, typename U>
LineageTreeNode<T,U>* findRoot( LineageTreeNode<T,U>* lngNode ) {
    
    if ( lngNode->parent == nullptr )
        return lngNode ;
    else
        return findRoot( lngNode->parent ) ;
    
}

template <typename T, typename U>
void findLeaves( std::unordered_set<LineageTreeNode<T,U>*>& leaves, LineageTreeNode<T,U>* node ) {
    
    if ( node->children.size() == 0 ) {
        
        //if ( node->parent != nullptr ) // root node can not be leaf
        leaves.insert( node ) ; // root node can be a leaf if it is the only node
        
    }
    else {
        
        for ( auto child : node->children )
            findLeaves( leaves, child ) ;
        
    }
    
}

template <typename T, typename U>
void removeRedundantNodeMerge( LineageTreeNode<T,U>* midNode, const std::vector<T>& sampledLngs ) {
    
    if ( !midNode )
        return ;
    
    if ( midNode->children.size() == 1 ) { // may merge if has one child only
    
        // check if node is sampled
        bool isSampled = false ;
        for ( auto& lng : sampledLngs ) {
            
            if ( lng == midNode->lng ) {
            
                isSampled = true ;
                break ;
            
            }
        
        }
        if ( !isSampled ) {  // if not sampled, remove
            
            if ( midNode->parent == nullptr ) { // if root node
                
                midNode->children[0]->parent = nullptr ;
                
            }
            else {
                
                midNode->children[0]->parent = midNode->parent ;
                midNode->parent->eraseChild( midNode ) ;
                midNode->parent->children.push_back( midNode->children[0] ) ;
                removeRedundantNodeMerge( midNode->parent, sampledLngs ) ;

                
            }
            
            delete midNode ;
            
        }
        else { // recurse up
            
            if ( midNode->parent != nullptr ) // recurse up until root
                removeRedundantNodeMerge( midNode->parent, sampledLngs ) ;
            
        }
        
    }
    else { // recurse up
        
        if ( midNode->parent != nullptr ) // recurse up until root
            removeRedundantNodeMerge( midNode->parent, sampledLngs ) ;
        
    }
    
}

template <typename T, typename U>
void printEdges( LineageTreeNode<T,U>* root ) {

    assert( root->parent == nullptr ) ;
    std::cout << "@->" << utils::to_string( root->lng ) <<  " t: " << root->t << "\n" ;
    
    for ( auto child : root->children ) {
        
        std::cout << utils::to_string( root->lng ) << "->" << utils::to_string( child->lng ) << " t: " << child->t << "\n" ;
        printEdgesChildren( child ) ;
        
    }
    
}

template <typename T, typename U>
void printEdgesChildren( LineageTreeNode<T,U>* lngNode ) ;


#endif /* LineageTree_hpp */
