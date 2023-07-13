//
//  point.cpp
//  CBM
//
//

#include "point.hpp"


namespace Geom {
    
    //===== Point =====//

    Point::Point(): x( std::numeric_limits<double>::min() ), y ( std::numeric_limits<double>::min() ) {};
    Point::Point( const double& x, const double& y ): x( x ), y( y ) {};

    bool Point::operator==( const Point& other ) {
        
        if ( this->x != other.x )
            return false;
        
        if ( this->y != other.y )
            return false;
        
        return true;
        
    }

    //===== PointObj =====//

    template <typename T>
    PointObj<T>::PointObj( const double& x, const double& y, const T& data_ ): point( x, y ), data( data_ ) {};

    template <typename T>
    bool PointObj<T>::operator==( const PointObj<T>& other ) {
        if ( this->point == other.point )
            return true;
    }


    double euclidean_distance( const Point& p, const Point& q ) {
        return sqrt( pow( p.getX() - q.getX(), 2. ) + pow( p.getY() - q.getY(), 2. ) );
    }

}
