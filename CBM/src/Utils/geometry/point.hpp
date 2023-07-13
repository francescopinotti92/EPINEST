//
//  point.hpp
//  CBM
//
//

#ifndef point_hpp
#define point_hpp

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <iostream>
#include <limits>
#include <math.h>

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

namespace Geom {
    
    /*
     A point in the 2D plane (cartesian coordinates)
     */
    class Point {
    public:
        Point();
        Point( const double& x, const double& y );
        bool operator==( const Point& other );
        double getX() const               { return x; }
        double getY() const               { return y; }
        void setX(double x_)              { x = x_; }
        void setY(double y_)              { y = y_; }
    private:
        double x;
        double y;
    };


    template <typename T>
    struct PointObj {
        PointObj( const double& x, const double& y, const T& data );
        bool operator==( const PointObj<T>& other );
        double x() const { return point.getX() ; }
        double y() const { return point.getY() ; }
        Point point;
        T data;
    };
    
    double euclidean_distance( const Point& p, const Point& q );

}

// adapt Position object to boost::geometry
BOOST_GEOMETRY_REGISTER_POINT_2D_GET_SET( Geom::Point, double,
                                         bg::cs::cartesian,
                                         getX, getY, setX, setY)

#endif /* point_hpp */
