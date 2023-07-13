//
//  point_manager.cpp
//  CBM
//
//

#include "point_manager.hpp"


namespace Geom {

    template <typename T>
    PointManager<T>::PointManager() {};

    template <typename T>
    void PointManager<T>::addPoint( Point pos, T data ) {
        rtree.insert( std::make_pair( pos, data ) );
    }

    // returns a vector of points within distance 'radius_km' (in km) from 'pos'.
    template <typename T>
    std::vector<std::pair<Point, T>> PointManager<T>::getNodesWithinRadius( const Point& pos, const double& radius_km ) {
        
        if ( radius_km == 0. )
            return {}; // no other points
        
        std::vector<std::pair<Point, T>> res;                        // this will store results
        res.reserve( 100 );
            
        double radius_m = radius_km * 1000. ;
        
        rtree.query( bgi::satisfies([&]( const std::pair<Point, T>& pt ) { return ( euclidean_distance( pt.first, pos ) < radius_m ); } ), std::back_inserter( res ) );
        
        return res;
    }

    // As above, but also skips points holding 'skipValue' as a data member.
    template <typename T>
    std::vector<std::pair<Point, T>> PointManager<T>::getNodesWithinRadius( const Point& pos, const double& radius_km, T skipValue ) {
        
        if ( radius_km == 0. )
            return {}; // no other points
        
        std::vector<std::pair<Point, T>> res;                        // this will store results
        res.reserve( 100 );
            
        double radius_m = radius_km * 1000. ;
        
        rtree.query( bgi::satisfies([&]( const std::pair<Point, T>& pt ) { return ( euclidean_distance( pt.first, pos ) < radius_m ) and ( pt.second != skipValue ); } ), std::back_inserter( res ) );
        
        return res;
    }

    template <typename T>
    bgi::rtree<std::pair<Point, T>, bgi::quadratic<16> >& PointManager<T>::getPoints() {
        return rtree;
    }

    // explicit instantiation
    template class PointManager<uint>;

}
