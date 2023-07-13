//
//  IPF.cpp
//  CBM
//
//

#include "IPF.hpp"

namespace net_utils {

void IPF(w_adjacency_list& adj,
         const std::vector<double>& psum_r,
         const std::vector<double>& psum_c,
         const uint& maxIter,
         const double& convergenceThreshold,
         const double& fillZero) {

        
    {
        double sum_r = std::accumulate( std::begin( psum_r ), std::end( psum_r ) , 0. );
        double sum_c = std::accumulate( std::begin( psum_c ), std::end( psum_c ) , 0. );
        
        assert( std::fabs(sum_r - sum_c) < 1. );
        
    }
    
    // compute number of nodes (row and col)
    uint nNodes_r = static_cast<uint>( psum_r.size() );
    uint nNodes_c = static_cast<uint>( psum_c.size() );
    
    std::vector<double> psum_r_tmp( psum_r.size(), 0. );
    std::vector<double> psum_c_tmp( psum_c.size(), 0. );
    
    std::vector<double> x_r( psum_r.size(), 0. );  // a_i
    std::vector<double> x_c( psum_c.size(), 1. );  // b_j
    
    // fill 'zeros' in initial matrix with a positive default value
    // this improves convergence
    // avoid adding elements in rows/cols with zero marginal sums
    if ( fillZero > 0. ) {
        for ( uint i = 0; i < nNodes_r; ++i ) {
            if ( psum_r[i] == 0. )
                continue;
            else {
                for ( uint j = 0; j < nNodes_c; ++j ) {
                    if ( psum_c[j] > 0. ) {
                        if ( !adj.hasNbr( i, j ) )
                            adj.addNbr( i, j, fillZero );
                    }
                }
            }
        }
    }
    
    // test: write output
    
    /*
    std::ofstream writeTmp;
    writeTmp.open("/Users/user/Documents/Project_simulations/IPFtest.txt");
    for (auto el: psum_r)
        writeTmp << "r," << el << std::endl;
    for (auto el: psum_c)
        writeTmp << "c," << el << std::endl;
    
    for ( int i = 0; i < nNodes_r; ++i ) {
        for (auto& nb: adj[i] ) {
            writeTmp << "e," << i << "-" << nb.id << "-" << nb.weight << std::endl;
        }
    }
    writeTmp.close();
     */
    
    
    double distL2 = 0.;
    uint nStep = 0;
    
    // compute in-degree column nodes
    std::vector<uint> k_in_c( nNodes_c, 0 );
    for ( uint i = 0; i < nNodes_r; ++i ) {
        for ( nbrW& nb: adj[i] ) {
            k_in_c[nb.id]++;
        }
    }
    
    while( nStep < maxIter ) {
        distL2 = 0.;
        
        // update a's
        for ( uint i = 0; i < nNodes_r; ++i ) {
            if ( adj[i].size() > 0 ) {
                double tmp = 0.;
                for ( nbrW& nb: adj[i] ) {
                    tmp += x_c[nb.id] * nb.weight;
                }
                distL2 += pow( x_r[i] - ( psum_r[i] / tmp ), 2. );
                x_r[i] = psum_r[i] / tmp;
            }
        }
        
        // precompute factors to update b's
        std::vector<double> tmp(nNodes_c, 0.);
        for ( uint i = 0; i < nNodes_r; ++i ) {
            for ( nbrW& nb: adj[i] ) {
                tmp[nb.id] += x_r[i] * nb.weight;
            }
        }
        
        // update b's
        for ( uint i = 0; i < nNodes_c; ++i ) {
            if ( k_in_c[i] > 0 ) {
                distL2 += pow( x_c[i] - ( psum_c[i] / tmp[i] ), 2. );
                x_c[i] = psum_c[i] / tmp[i];
            }
        }
        
        //std::cout << distL2 << std::endl;
        
        // check if algorithm converged!
        if ( distL2 < convergenceThreshold ) {
            for ( uint i = 0; i < nNodes_r; ++i ) {
                for ( nbrW& nb: adj[i] ) {
                    nb.weight *= x_r[i] * x_c[nb.id];  // update the matrix result
                }
            }
            break;
        }
        
        //std::cout << distL2 << std::endl;
        
        nStep++;
    }
    //std::cout << "end IPF" << std::endl;
    
    if ( nStep == maxIter ) {
        //throw Warning("IPF procedure reached maximum number of iterations\nL2 dist: " + std::to_string(distL2));
        std::cout << "IPF procedure reached maximum number of iterations\nL2 dist: " + std::to_string(distL2) << std::endl;
    }
}

}
