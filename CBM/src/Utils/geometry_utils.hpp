//
//  geometry_utils.hpp
//  CBM
//
//

#ifndef geometry_utils_hpp
#define geometry_utils_hpp

#include "baseNode.hpp"
#include "random.hpp"
#include "types.hpp"
#include "distance_kernels.hpp"
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <vector>

/*
 IMPORTANT:
 All points are specified in geographic coordinates (longitude and latitude), measured in degrees
 longitude goes from -180 to 180
 latitude goes from -90 to 90
*/

class BaseNode;

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

typedef bg::model::point<double, 2, bg::cs::spherical_equatorial<bg::degree>> GeoPoint;   // basic point type (lon, lat)
typedef bg::model::box<GeoPoint> GeoBox;                                        // rectangular box
typedef bg::model::polygon<GeoPoint> GeoPolygon;                                // closed polygon

typedef bg::srs::spheroid<double> stype ; // define sphere
//typedef bg::strategy::distance::haversine<stype> haversine_type ;
typedef bg::strategy::distance::vincenty<stype> vincenty_type;


//std::vector<GeoPoint>* getRandomCoords( const GeoPolygon& poly, const uint& Npts, Random& randGen );

//====== Position ======//

struct Position {
    Position() {
        setLon(0.);
        setLat(0.);
        
    }
    Position( double lon, double lat ) {
        setLon(lon);
        setLat(lat);
    }
    Position( const Position& pos ) {
        setLon( pos.getLon() );
        setLat( pos.getLat() );
    }
    Position( GeoPoint coords_ ): coords(coords_) {};
    
    GeoPoint getCoords() const          { return coords; }
    double getLon() const               { return bg::get<0>(coords); }
    double getLat() const               { return bg::get<1>(coords); }
    void setLon(double lon)             { bg::set<0>(coords, lon); }
    void setLat(double lat)             { bg::set<1>(coords, lat); }
    
    GeoPoint coords;
};

double distanceKm( const Position& pos1, const Position& pos2 );

// adapt Position object to boost::geometry
BOOST_GEOMETRY_REGISTER_POINT_2D_GET_SET(Position, double,
                                         bg::cs::spherical_equatorial<bg::degree>,
                                         getLon, getLat, setLon, setLat)

// Now it is possible to use Position in boost::geometry as an element (the first) in customPoint pair
typedef std::pair<Position, uint> customPoint;
typedef std::vector<Position*> pPosVec;
typedef std::vector<Position> PosVec;


//====== SpatialPointManager ======//

class SpatialPointManager {
public:
    SpatialPointManager();
    ~SpatialPointManager() = default;
    void addPoints(pPosVec& poss);
    void addPoint(Position pos, uint id);
    void addPoint(Position* pos, uint id);
    std::vector<customPoint> getNodesWithinRadius(const Position& pos, const double& radius_km, uint check_id = 1000000000);
    std::vector<customPoint> getNodesWithinRadius(Position* pos, const double& radius_km, uint check_id = 1000000000);
    std::vector<customPoint> getNodesNearest(Position* pos, uint npts, uint check_id = 1000000000);
    bgi::rtree<customPoint, bgi::quadratic<16> >& getPoints();

private:
    bgi::rtree<customPoint, bgi::quadratic<16> > rtree;
};



#endif /* geometry_utils_hpp */
