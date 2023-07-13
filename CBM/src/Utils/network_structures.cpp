//
//  network_structures.cpp
//  CBM
//
//

#include "network_structures.hpp"


namespace net_utils {

//=== weighted adjacency list ===//

w_adjacency_list::w_adjacency_list() {};

w_adjacency_list::w_adjacency_list( const uint& Nnodes_, const uint& initialCapacity ): Nnodes(Nnodes_) {
    adj_list = {};
    adj_list.reserve( Nnodes );
    for ( uint i = 0; i < Nnodes; ++i ) {
        adj_list.push_back( {} );
        adj_list[i].reserve( initialCapacity );
    }
}

void w_adjacency_list::addNbr( const uint &id1, const uint &id2, const double &weight ) {
    
    bool has_neighbor = false;
    for ( auto& nbr: adj_list[id1] ) {
        
        if ( nbr.id == id2 ) {
            nbr.weight = weight;
            has_neighbor = true;
            break;
        }
        
    }
    if ( has_neighbor == false )
        adj_list[id1].push_back( nbrW( id2, weight ) );
}

// fill adjacency list with edges from another adjacency list
// does not allow for multi-edges
void w_adjacency_list::addNbrsFromAdjList( w_adjacency_list& otherList ) {
    
    if ( otherList.Nnodes != this->Nnodes )
        return;
    
    for ( uint i = 0, nNodesOther = otherList.Nnodes; i < nNodesOther; ++i ) {
        auto& nbrs = otherList[i];
        for ( auto& nbr: nbrs ) {
            if ( !this->hasNbr( i, nbr.id ) ) {
                this->addNbr( i, nbr.id, nbr.weight );
            }
        }
    }
}

bool w_adjacency_list::hasNbr( const uint &id1, const uint &id2 ) {
    for ( auto& nbr: adj_list[id1] ) {
        if ( nbr.id == id2 )
            return true;
    }
    return false;
}

void w_adjacency_list::writeToFile( std::string fileName ) {
    
    std::ofstream writeFile( fileName );
    
    writeFile << "id1 id2 weight" << std::endl;
    
    for ( uint i = 0; i < Nnodes; ++i ) {
        auto& adjListTmp = adj_list[i];
        for ( auto& elem: adjListTmp ) {
            writeFile << i << " " << elem.id << " " << elem.weight << std::endl;
        }
    }
    
    writeFile.close();
    
}


//=== weighted edge list ===//

w_edge_list::w_edge_list(): Nedges(0) {};

w_edge_list::w_edge_list(const uint& initialCapacity): Nedges(0) {
    edg_list.reserve( initialCapacity );
}

void w_edge_list::addEdge(const uint &id1, const uint &id2, const double &weight) {
    edg_list.emplace_back( id1, id2, weight );
    Nedges++;
}

void w_edge_list::writeToFile( std::string fileName ) {
    
    std::ofstream writeFile( fileName );
    
    writeFile << "id1 id2 weight" << std::endl;
    
    for ( auto& elem: edg_list ) {
        writeFile << elem.id1 << " " << elem.id2 << " " << elem.weight << std::endl;
    }
    
    writeFile.close();
    
}


//=== unweighted adjacency list ===//

uw_adjacency_list::uw_adjacency_list() {};

uw_adjacency_list::uw_adjacency_list( const uint& Nnodes_, const uint& initialCapacity ): Nnodes(Nnodes_) {
    adj_list.resize(Nnodes, std::vector<uint>() );
    
    for (auto& nbr_list: adj_list)
        nbr_list.reserve( initialCapacity );
}

void uw_adjacency_list::addNbr( const uint &id1, const uint &id2 ) {
    if ( !hasNbr( id1, id2 ) )
        adj_list[id1].push_back( id2 );
}

bool uw_adjacency_list::hasNbr( const uint &id1, const uint &id2 ) {
    for ( auto& nbr: adj_list[id1] ) {
        if ( nbr == id2 )
            return true;
    }
    return false;
}


} // end of namespaxe
