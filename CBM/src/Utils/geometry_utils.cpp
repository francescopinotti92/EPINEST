//
//  geometry_utils.cpp
//  CBM
//
//

#include "geometry_utils.hpp"


// free function to compute distance between two points
double distanceKm( const Position& pos1, const Position& pos2 ) {
    //return bg::distance( pos1, pos2, haversine_type(R_EARTH_KM) );
    return bg::distance( pos1, pos2, vincenty_type() ) / 1000.; // Uses vincenty formula and converts from m to km
}

//====== SpatialPointManager ======//

// constructor
SpatialPointManager::SpatialPointManager() {};

// add point from Position object
void SpatialPointManager::addPoint( Position pos, uint id ) {
    rtree.insert(std::make_pair(pos, id));
}

void SpatialPointManager::addPoint( Position* pos, uint id ) {
    rtree.insert(std::make_pair(*pos, id));
}

// add multiple points from a single vector
void SpatialPointManager::addPoints( pPosVec& poss ) {
    uint id = 0;
    for ( Position* pos: poss ) {
        addPoint( *pos, id );
        id++;
    }
}

// returns a vector of pointers to nodes within distance 'radius_km' (in km) from 'node'
// N.B. uses haversine distance on a sphere with radius defined in macro 'R_EARTH_KM'
std::vector<customPoint> SpatialPointManager::getNodesWithinRadius( const Position& pos, const double& radius_km, uint check_id ) {
    
    if (radius_km == 0.)
        return {};
    
    std::vector<customPoint> res;                        // this will store results
    res.reserve( 100 );
    
    GeoPoint target = pos.getCoords();       // get coords point under examination
    
    if ( check_id == 1000000000 ) {
        // check distance, discard same node
        //rtree.query(bgi::satisfies([&](customPoint const& pt) {return (bg::distance(pt.first, target, vincenty_type() ) < radius_km) and ( pt.second != check_id );}), std::back_inserter(res) );
        rtree.query(bgi::satisfies([&](customPoint const& pt) { return ( distanceKm( pt.first, target ) < radius_km ) and ( pt.second != check_id ); } ), std::back_inserter( res ) );
    }
    else {
        // check distance only
        //rtree.query(bgi::satisfies([&](customPoint const& pt) {return bg::distance(pt.first, target, vincenty_type() ) < radius_km ;}), std::back_inserter(res) );
        rtree.query(bgi::satisfies([&](customPoint const& pt) {return distanceKm( pt.first, target ) < radius_km; } ), std::back_inserter( res ) );
    }
    
    return res;
}


std::vector<customPoint> SpatialPointManager::getNodesWithinRadius( Position* pos, const double& radius_km, uint check_id ) {
    
    if (radius_km == 0.)
        return {};
    
    std::vector<customPoint> res;                        // this will store results
    res.reserve(100);
    
    GeoPoint target = pos->getCoords();       // get coords point under examination
    
    
    if ( check_id == 1000000000 ) {
        // check distance, discard same node
        rtree.query( bgi::satisfies([&]( customPoint const& pt ) { return ( bg::distance( pt.first, target, vincenty_type() ) < radius_km ) and ( pt.second != check_id ); } ), std::back_inserter( res ) );
    }
    else {
        // check distance only
        rtree.query( bgi::satisfies([&]( customPoint const& pt ) { return bg::distance( pt.first, target, vincenty_type() ) < radius_km; } ), std::back_inserter( res ) );
    }
    
    return res;
}

// returns a vector of pointers to nodes that are nearest to 'node'
// N.B. uses haversine distance on a sphere with radius defined in macro 'R_EARTH_KM'
std::vector<customPoint> SpatialPointManager::getNodesNearest( Position* pos, uint npts, uint check_id ) {
    
    if ( npts <= 0 )
        return {};
    
    std::vector<customPoint> res;                        // this will store results
    res.reserve(npts);
    
    GeoPoint target = pos->getCoords();       // get coords point under examination
    
    rtree.query( bgi::nearest( target, npts ), std::back_inserter( res ) );
    
    return res;
}

// return rtree so that it is possible to iterate over points
bgi::rtree<customPoint, bgi::quadratic<16> >& SpatialPointManager::getPoints() {
    return rtree;
}


// returns a vector of Npts points uniformly sampled within a closed polygon

/*
std::vector<GeoPoint>* getRandomCoords(const GeoPolygon& poly, const uint& Npts, Random& randGen) {
    std::vector<GeoPoint>* res = new std::vector<GeoPoint>( {} );
    res->reserve(Npts);
    
    GeoBox bbox;
    bg::envelope(poly, bbox);  // get box covering poly
    
    // obtain min and max long and lat from box
    double min_lo = bg::get<bg::min_corner, 0>(bbox);
    double min_la = bg::get<bg::min_corner, 1>(bbox);
    double max_lo = bbox.max_corner().get<0>();
    double max_la = bbox.max_corner().get<1>();
    
    // transform lat values
    // convert to radians and shift to [0,pi] interval
    double aa = (min_la * M_PI / 180.) + M_PI_2;
    double bb = (max_la * M_PI / 180.) + M_PI_2;
    
    // sample point uniformly on a sphere, in the region delimited by bbox
    uint nSuccess = 0;
    while (nSuccess < Npts) {
       
        double lo = min_lo + (max_lo - min_lo) * randGen.getUni();
        double la = ( acos( - randGen.getUni() * ( cos(aa) - cos(bb) ) + cos(aa) ) - M_PI_2 ) * 180. / M_PI;
        GeoPoint pp(lo, la);
        
        if ( bg::within(pp, poly) ) { // accept only if point within polygon
            res->push_back(pp);
            nSuccess++;
        }
    }
    
    return res;
}
 */
