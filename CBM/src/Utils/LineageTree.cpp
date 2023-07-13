//
//  LineageTree.cpp
//  CBM
//
//

#include "LineageTree.hpp"

namespace utils {
   
using std::to_string ;

    std::string to_string(const DataLineage& x) {

       return fmt::format( "{}", x.loc ) ;

    }

}

/*
 Formats nodes attributes according to NHX formatting rules
 Attributes:
 - loc : location of infection (where the lineage was born)
 */
std::string convertData2NHX( const DataLineage& data ) {

    return fmt::format( "loc={}", data.loc ) ;
    
}

//====== LineageTreeNode ======//

template <typename T, typename U>
LineageTreeNode<T,U>::LineageTreeNode( const T& lng, const U& data, const uint& t, const bool& extant, LineageTreeNode<T,U>* parent ): t( t ), tSample( 0 ), locSample("NA"), lng( lng ), data( data ), extant( extant ), parent( parent ), needed( false ), sampled( false ) {
    
    children = {} ;
    
}

template<typename T, typename U>
void LineageTreeNode<T,U>::eraseChild( LineageTreeNode<T,U>* child ) {
    
    auto it = children.begin() ;
    while( it != children.end() ) {
        
        if ( (*it)->lng == child->lng ) {
            std::swap( *it, children.back() ) ;
            children.pop_back() ;
            break ;
        }
        ++it ;
        
    }
    
}

template<typename T, typename U>
uint LineageTreeNode<T,U>::getSizeChildren() {
    
    return static_cast<uint>( children.size() ) ;
    
}


//====== LineageTree ======//

template <typename T, typename U, class Hash>
LineageTree<T,U,Hash>::LineageTree(): nnodes( 0 ) {
    
    extantLngs = {} ;
    roots.clear() ;
    
}

template <typename T, typename U, class Hash>
void LineageTree<T,U,Hash>::reset() {

    auto extantLngsCopy = extantLngs ; // Use copy to avoid errors during lineage removal
    for ( auto leaves : extantLngsCopy ) {
        
        T lng = leaves.first ;
        removeExtantLineage( lng ) ;
        
    }
    
}


template <typename T, typename U, class Hash>
void LineageTree<T,U,Hash>::addExtantLineage( const uint& t, const T& lng, const U& data, const T& lngParent ) {
    
    LineageTreeNode<T,U>* lngParentNode = extantLngs[lngParent] ;
    LineageTreeNode<T,U>* lngNode = new LineageTreeNode<T,U>( lng, data, t, true, lngParentNode ) ;
    lngParentNode->children.push_back( lngNode ) ;
    extantLngs[lng] = lngNode ;
    ++nnodes ;
    
}

template <typename T, typename U, class Hash>
void LineageTree<T,U,Hash>::addExtantLineageExternal( const uint& t, const T& lng, const U& data ) {

    LineageTreeNode<T,U>* lngNode = new LineageTreeNode<T,U>( lng, data, t, true, nullptr ) ;
    extantLngs[lng] = lngNode ;
    roots.insert( lngNode ) ;
    ++nnodes ;
    
}


/*
 Remove a lineage that was extant but is not anymore.
 This could be a tip or an intermediate node.
 
 If lineage has not been sampled already, do the following:
 
 1. Check if lng is tip or intermediate
    a. if TIP, check if lng is root
        i. if YES, then remove it from root list
        ii. if NOT, notify parent about lng removal ( 'notifyParent' )
    b. if INTERMEDIATE with single child, prune current node
       and attach child to parent ( 'mergeParent').
 
 */
template <typename T, typename U, class Hash>
void LineageTree<T,U,Hash>::removeExtantLineage( const T& lng ) {
    
    LineageTreeNode<T,U>* lngNode = extantLngs[lng] ;
    lngNode->extant = false ;
    
    if ( !lngNode->sampled  ) { // remove lineage only if sampled
        
        if ( ( lngNode->children ).size() == 0 ) { // has no extant children
            
            if ( lngNode->parent != nullptr ) // if parent is not root, broadcast removal upstream
                notifyParent( lngNode->parent, lngNode ) ;
            else
                roots.erase( lngNode ) ; // remove from root
            
            
            delete lngNode ;
            --nnodes;
            
        }
        
        // check if merge is possible
        else if ( ( lngNode->children ).size() == 1 )
            mergeParentChild( lngNode ) ;
        
    }
    
    extantLngs.erase( lng ); // lng is not extant anymore, hence update extant list
    
}

template <typename T, typename U, class Hash>
void LineageTree<T,U,Hash>::sampleExtantLineage( const T& lng, const uint& t, const std::string& locSample ) {
    
    extantLngs[lng]->sampled = true ;
    extantLngs[lng]->tSample = t ;
    extantLngs[lng]->locSample = locSample ;

}

/*
 Prunes transmission tree and yields the reduced transmission tree
 leading to a handful of lineages sampled at time t
 */
template <typename T, typename U, class Hash>
std::vector<LineageTreeNode<T,U>*> LineageTree<T,U,Hash>::subSampleTree( const std::unordered_map<T,DataLineageSampling,Hash>& sampledLngsInfo ) {
    
    // check that sampled lineages are extant
    for ( auto& lngInfo : sampledLngsInfo ) {
        
        auto& lng = lngInfo.first ;
        
        if ( extantLngs.find( lng ) == extantLngs.end() )
            assert( false ) ; // ?? improve error log
        
        // sample chicken (also add sampling details)
        sampleExtantLineage( lng, lngInfo.second.tSample, lngInfo.second.locSample ) ;
        
    }
    
    // assign sampled lineages to root nodes
    std::unordered_map<T, T, Hash> lng2tree ;
    if ( roots.size() == 0 ) { // simplified procedure if only one child
        
        for ( auto& lngInfo : sampledLngsInfo )
            lng2tree[lngInfo.first] = (*roots.begin())->lng ;
        
    }
    else {
        
        for ( auto& lngInfo : sampledLngsInfo )
            lng2tree[lngInfo.first] = getRootNode( extantLngs[ lngInfo.first ] )->lng ;
            
    }
    
    // get roots found
    std::unordered_set<T, Hash> rootsFound = {} ;
    
    for ( auto& tmp : lng2tree )
        rootsFound.insert( tmp.second ) ;
        
    
    // find subtree for each root found
    std::vector<LineageTreeNode<T,U>*> res = {} ;
    for ( T root : rootsFound ) {
        
        // find sampled lineages with same ancestor
        std::vector<T> selectedLngs = {} ;
        for ( auto& tmp : lng2tree ) {
            
            if ( tmp.second == root )
                selectedLngs.push_back( tmp.first ) ;
            
        }
        
        
        // Apply recursive algorithm to homologous set of lineages
        
        LineageTreeNode<T, U>* rootNode = getRootNode( extantLngs[ selectedLngs[0] ] ) ;

        markNodeNeeded( rootNode, selectedLngs ) ;
        
        // extract subtree..
        
        LineageTreeNode<T, U>* subTreeRoot = extractSubTree( rootNode, nullptr ) ;
        subTreeRoot = eliminateRedundantNodes( subTreeRoot, selectedLngs ) ;
        
        // ??? set needed to false in original tree (needed if multiple sampling)
        
        res.push_back( subTreeRoot ) ;
        
    }
    
    return res ;
    
}

/*
 Prunes transmission tree and yields the reduced transmission tree
 leading to a handful of lineages that have been previously sampled.
 */
template <typename T, typename U, class Hash>
std::vector<LineageTreeNode<T,U>*> LineageTree<T,U,Hash>::subSampleTree() {
    
    // loop over root nodes
    std::vector<LineageTreeNode<T,U>*> res = {} ;
    for ( LineageTreeNode<T,U>* rootNode : roots ) {
        
        // find sampled lngs descending from root
        std::vector<T> selectedLngs = getSampledLineages( rootNode );
        
        uint nSampledLngs = static_cast<uint>( selectedLngs.size() ) ;
        
        if ( nSampledLngs > 0 ) {
            
            // extract subtree..
            markNodeNeeded( rootNode, selectedLngs ) ;
            LineageTreeNode<T, U>* subTreeRoot = extractSubTree( rootNode, nullptr ) ;
            subTreeRoot = eliminateRedundantNodes( subTreeRoot, selectedLngs ) ;
            res.push_back( subTreeRoot ) ;
            
        }

    }
    
    return res ;
    
}




// extract subtree using the 'needed' attribute
// starting from a current tree node, 'node', and attach node to 'parent', which is a newly created node
// that builds the output, subsampled tree
template <typename T, typename U, class Hash>
LineageTreeNode<T,U>* LineageTree<T,U,Hash>::extractSubTree( LineageTreeNode<T,U>* node, LineageTreeNode<T,U>* parent ) {
   
    assert( node != nullptr ) ;
    
    // copy original node into new node (output)
    LineageTreeNode<T,U>* newNode = new LineageTreeNode<T, U>( node->lng, node->data, node->t, node->extant, parent ) ;
    
    // Add rest of sampling information
    newNode->sampled = node->sampled ;
    newNode->tSample = node->tSample ;
    newNode->locSample = node->locSample ;
    
    for ( auto& child : node->children ) {
        
        if ( child->needed ) {
            
            LineageTreeNode<T,U>* newChild = extractSubTree( child, newNode ) ;
            ( newNode->children ).push_back( newChild ) ;
            
        }
        
    }
    
    return newNode ;
    
}



/*
 Set selected nodes as 'needed' if they are to be included in
 the reduced transmission tree.
 N.B. 'neededLngs' contains a subset of sampled nodes and their
 ancestors limited to a single tree component
 */
template <typename T, typename U, class Hash>
bool LineageTree<T,U,Hash>::markNodeNeeded( LineageTreeNode<T,U>* node, const std::vector<T>& neededLngs ) {
    
    if ( node->extant ) { // is extant
        
        bool needed = false ;
        for ( const auto& ll : neededLngs ) {
            
            if ( ll == node->lng ) {
                
                needed = true ;
                break ;
                
            }
        
        }
        
        if ( needed ) { // if sampled
            node->needed = true ;
            for ( auto& child : node->children )
                markNodeNeeded( child, neededLngs ) ;
        }
        else { // if not sampled directly
            
            for ( auto& child : node->children ) {
                
                bool isChildNeeded = markNodeNeeded( child, neededLngs ) ;
                node->needed = ( node->needed or isChildNeeded ) ;
                
            }
            
        }
        
        return node->needed ;
    
    }
    else { // is not extant, check if needed due to children
        
        node->needed = false ;
        for ( auto& child : node->children ) {
            
            bool isChildNeeded = markNodeNeeded( child, neededLngs ) ;
            node->needed = ( node->needed or isChildNeeded ) ;
            
        }
        
    }
    
    return node->needed ;
    
}

/*
 Retrieve all lineaged descended from 'rootNode' that have been marked as sampled.
 May include both extant and extinct species.
 */
template <typename T, typename U, class Hash>
std::vector<T> LineageTree<T,U,Hash>::getSampledLineages( LineageTreeNode<T,U>* rootNode ) {
    
    // return empty vector if not root
    if ( rootNode->parent != nullptr )
        return {} ;
    
    // find all sampled lineages recursively
    std::vector<T> sampledLngs = {} ;
    getSampledLineagesRecursive( rootNode, sampledLngs ) ;
    
    return sampledLngs ;
    
}


/*
 Fills 'lngs' vector with sampled lineages in a recursive manner
 */
template <typename T, typename U, class Hash>
void LineageTree<T,U,Hash>::getSampledLineagesRecursive( LineageTreeNode<T,U>* node, std::vector<T>& lngs ) {
    
    if ( node->sampled ) // if sampled, add node to vector
        lngs.push_back( node->lng ) ;
    
    for ( auto child : node->children )
        getSampledLineagesRecursive( child, lngs ) ;
    
}




/*
 Notifies 'parent' that 'child' lineage went extinct
 */

/*
template <typename T, typename U, class Hash>
void LineageTree<T,U,Hash>::notifyParent( LineageTreeNode<T,U>* parent, LineageTreeNode<T,U>* child ) {
        
    if ( parent->extant ) { // if is extant remove 'child' only
        
        if ( !child->sampled )
            parent->eraseChild( child ) ;
        
    }
    else { // if not extant
        
        // if parent removes its only child, remove parent
        if ( ( parent->children ).size() == 1 ) { 
            if ( parent->parent != nullptr ) // notify parent upstream
                notifyParent( parent->parent, parent ) ;
            else
                roots.erase( parent ) ; // is root, delete from roots
            delete parent ;
            --nnodes ;
            
        } // just remove child. Keep parent because it could have extant children
        else {
            
            parent->eraseChild( child ) ;
            
            // if only one child left, remove one node
            if ( ( parent->children ).size() == 1 )
                mergeParentChild( parent ) ;
            
        }
        
}*/

/*
 Notifies 'parent' that 'child' lineage went extinct. Also broadcasts
 signal recursively.
 Depending on status of 'parent' and whether 'child' was sampled or not,
 decides whether to prune redundant leaves (=> do removal) or redundant
 mid nodes (=> do merge).
 
 
 - Remove 'child' if it was not sampled
 - Proceed only if parent is extinct
 - Check if parent is sampled:
    1. If YES, do nothing
    2. If NOT, check number of children:
        a. If childless, check if root:
            i.  If NOT, notify grandparent node
            ii. If YES, remove from root list
            iii. In both cases, free memory and update nnodes
        b. If only one child, do merge move
        c. Else, do nothing
 
 */
template <typename T, typename U, class Hash>
void LineageTree<T,U,Hash>::notifyParent( LineageTreeNode<T,U>* parent, LineageTreeNode<T,U>* child ) {
    
    bool parentExtinct  = !parent->extant   ;
    bool parentSampled  = parent->sampled   ;
    bool childSampled   = child->sampled    ;
    
    if ( !childSampled ) // erase child only if unsampled
        parent->eraseChild( child ) ;
    
 
    if ( parentExtinct ) {
        
        if ( !parentSampled ) {
            
            uint nChildren = parent->getSizeChildren() ;
            
            if ( nChildren == 0 ) {
                
                // parent is a redundant leaf node: remove
                bool parentRoot = parent->parent == nullptr ; //
                if ( !parentRoot ) {
                    
                    // notify grandparent
                    notifyParent( parent->parent, parent ) ;
                    
                }
                else {
                    // parent is also root: remove from root list
                    roots.erase( parent ) ;
                    
                }
                
                delete parent ; // free memory
                --nnodes ;
        
            }
            else if ( nChildren == 1 ) {
                // parent is a redundant mid node: do merge move
                mergeParentChild( parent ) ;
                
            }
            // else do nothing

        }
        
    }
        
}

/*
 Merges the only child of 'midNode' with its grandparent.
 Applies only to redundant midNodes (see notifyParent and
 comments below).
 */
template <typename T, typename U, class Hash>
void LineageTree<T,U,Hash>::mergeParentChild( LineageTreeNode<T,U>* midNode ) {
    
    // should not be called on nodes that:
    // 1. Have not exactly one child
    // 2. Are extant
    // 3. Have been sampled
    assert( midNode->children.size() == 1 ) ;
    assert( !midNode->extant ) ;
    assert( !midNode->sampled ) ;
    
    if ( midNode->parent != nullptr ) { // midNode is an intermediate node O->X->O
        
        midNode->children[0]->parent = midNode->parent ;
        midNode->parent->eraseChild( midNode ) ;
        midNode->parent->children.push_back( midNode->children[0] ) ;

    }
    else { // midNode is a root with a single child @->X->O
        
        midNode->children[0]->parent = nullptr ;
        roots.erase( midNode ) ;
        roots.insert( midNode->children[0] );
    
    }
    
    delete midNode ;
    --nnodes ;
    
}


template <typename T, typename U, class Hash>
uint LineageTree<T,U,Hash>::getSizeExtantLineages() {
    
    return static_cast<uint>( extantLngs.size() ) ;
    
}

template <typename T, typename U, class Hash>
uint LineageTree<T,U,Hash>::getSizeNodes() {
    
    return nnodes ;
    
}

template <typename T, typename U, class Hash>
void LineageTree<T,U,Hash>::printEdges() {
    
    for ( auto root : roots ) {
        
        std::cout << "@->" << utils::to_string( root->lng ) << "\n";
        printEdgesChildren( root ) ;
        
    }
    
}

template <typename T, typename U, class Hash>
void LineageTree<T,U,Hash>::printExtantLineages() {
    
    std::cout << "Extant lineages:\n" ;
    for ( auto& el : extantLngs )
        std::cout << utils::to_string( el.first )  << "\n" ;

    
}

template <typename T, typename U, class Hash>
LineageTreeNode<T,U>* LineageTree<T,U,Hash>::getRootNode( LineageTreeNode<T,U>* lngNode ) {
    
    if ( lngNode->parent == nullptr )
        return lngNode ;
    else
        return getRootNode( lngNode->parent ) ;
    
}




template <typename T, typename U, class Hash>
void LineageTree<T,U,Hash>::printParentChildEdge( LineageTreeNode<T,U>* child, bool recursive ) {

    std::string stringParent = "@";
    if ( child->parent != nullptr )
        stringParent = utils::to_string( child->parent->lng ) ;
        
    std::cout << stringParent << "->" << utils::to_string( child->lng )<< "\n";
    
    if ( recursive and child->parent != nullptr )
        printParentChildEdge( child->parent, true ) ;
    
}
    

// Helper functions

// takes a tree (rooted to 'node') and remove redundant intermediate nodes
// except those from 'sampledLngs'
    
template <typename T, typename U>
LineageTreeNode<T,U>* eliminateRedundantNodes( LineageTreeNode<T,U>* root, const std::vector<T>& sampledLngs ) {

    // find leaves
    std::unordered_set<LineageTreeNode<T,U>*> leaves = {} ;
    leaves.reserve( sampledLngs.size() ) ;

    findLeaves( leaves, root ) ; // find leaves
    
    // starting from each leaf, recurse back to root and remove intermediate leaves
    for ( auto leaf : leaves )
        removeRedundantNodeMerge( leaf->parent, sampledLngs );
        
    return findRoot( *( leaves.begin() ) ) ;
    
}


template <typename T, typename U>
void printEdgesChildren( LineageTreeNode<T,U>* lngNode ) {
    
    for ( auto& child : lngNode->children ) {
        
        std::cout << utils::to_string( lngNode->lng )  << "->" << utils::to_string( child->lng ) << " t: " << child->t << "\n" ;
        printEdgesChildren( child ) ;
        
    }
        
}




//====== Template definitions ======//

template class LineageTree<BirdID, DataLineage, hashBirdID>; // simple, just one integer ID and timing
