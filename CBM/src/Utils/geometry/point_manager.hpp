//
//  point_manager.hpp
//  CBM
//
//

#ifndef point_manager_hpp
#define point_manager_hpp

#include "point.hpp"
#include "types.hpp"
#include <boost/geometry/index/rtree.hpp>
#include <vector>


namespace Geom {

    template <typename T>
    class PointManager {
        typedef std::pair<Point, T> dataPoint;
    public:
        PointManager();
        ~PointManager() = default;
        void addPoint( Point pos, T id );
        
        std::vector<dataPoint> getNodesWithinRadius( const Point& pos, const double& radius_km );
        std::vector<dataPoint> getNodesWithinRadius( const Point& pos, const double& radius_km, T skipValue );
        bgi::rtree<dataPoint, bgi::quadratic<16> >& getPoints();
    private:
        bgi::rtree<dataPoint, bgi::quadratic<16> > rtree;
    };

}

#endif /* point_manager_hpp */
