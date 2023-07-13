//
//  network_structures.hpp
//  CBM
//
//

#ifndef network_structures_hpp
#define network_structures_hpp

#include <vector>
#include <fstream>
#include <string>

typedef uint32_t uint;


namespace net_utils {

// simple edge (i,j)
struct edge {
    edge(const uint& id1_, const uint& id2_): id1(id1_), id2(id2_) {};
    virtual ~edge() = default;
    uint id1;
    uint id2;
};

// weighted edge (i,j,w)
struct edgeW: public edge {
    edgeW(const uint& id1, const uint& id2, const double& weight_): edge(id1, id2), weight(weight_) {};
    double weight;
};

// weight neighbor info (j,w)
struct nbrW {
    nbrW(const uint& id_, const double& weight_): id(id_), weight(weight_) {};
    uint id;
    double weight;
};

//=== weighted adjacency list ===//
struct w_adjacency_list {
    w_adjacency_list();
    w_adjacency_list(const uint& Nnodes_, const uint& initialCapacity);
    void addNbr(const uint& id1, const uint& id2, const double& weight);
    void addNbrsFromAdjList( w_adjacency_list& otherList );
    bool hasNbr(const uint& id1, const uint& id2);
    
    std::vector<nbrW>& operator[](const uint& idx) { return adj_list[idx]; }

    void writeToFile( std::string fileName );

    std::vector<std::vector<nbrW>> adj_list;
    uint Nnodes;
};

//=== weighted edge list ===//
struct w_edge_list {
    w_edge_list();
    w_edge_list(const uint& initialCapacity);
    void addEdge( const uint& id1, const uint& id2, const double& weight );
    std::vector<edgeW>::iterator begin()  { return edg_list.begin(); }
    std::vector<edgeW>::iterator end()    { return edg_list.end(); }
    void writeToFile( std::string fileName );
    std::vector<edgeW> edg_list;
    uint Nedges;
};

//=== unweighted adjacency list ===//
struct uw_adjacency_list {
    uw_adjacency_list();
    uw_adjacency_list(const uint& Nnodes_, const uint& initialCapacity);
    void addNbr(const uint& id1, const uint& id2);
    bool hasNbr(const uint& id1, const uint& id2);
    
    std::vector<uint>& operator[](const uint& idx) { return adj_list[idx]; }
    
    std::vector<std::vector<uint>> adj_list;
    uint Nnodes;
};


}



#endif /* network_structures_hpp */
