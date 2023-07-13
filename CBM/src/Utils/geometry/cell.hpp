//
//  cell.hpp
//  CBM
//
//

#ifndef cell_hpp
#define cell_hpp

#include "point.hpp"
#include <vector>
#include <memory>


namespace Geom {
    
    struct Cell;
    typedef std::shared_ptr<Cell> pCell;

    /*
     A square cell in the 2D plane
     */
    struct Cell {
        Cell();
        Cell(  const double& lb_x, const double& lb_y, const double& side );
        bool contains_point( const Point& p );
        bool isWithinHRange( const Point& p );
        bool isWithinVRange( const Point& p );
        bool isEmpty() { return ( get_npoints() == 0 ) ? true : false ; }
        void addPoint( const Point& pt );
        void addPoints( const std::vector<Point>& pts );
        double computeDistanceFromCell( Cell other );

        
        inline double get_llx()                 { return ll.getX() ; }
        inline double get_lly()                 { return ll.getY() ; }
        inline double get_side()                { return side ; }
        inline int get_npoints()                { return static_cast<int>( pts.size() ); }
        std::vector<Point> getCorners()         { return corners; }
        std::vector<Point>& getInnerPoints()    { return pts; }

        Point ll, lr, ul, ur;
        double side;
        std::vector<Point> corners;
        std::vector<Point> pts;

    };

    template <typename T>
    struct CellObj {
        CellObj();
        CellObj(  const double& lb_x, const double& lb_y, const double& side );
        
        bool contains_point( const Point& p );
        bool isWithinHRange( const Point& p );
        bool isWithinVRange( const Point& p );
        template <typename U> bool contains_point( const PointObj<U>& p );
        template <typename U> bool isWithinHRange( const PointObj<U>& p );
        template <typename U> bool isWithinVRange( const PointObj<U>& p );
        bool isEmpty() { return ( get_npoints() == 0 ) ? true : false ; }
        void addPoint( const PointObj<T>& pt, const bool& check = true );
        
        inline int get_npoints()                { return static_cast<int>( pts.size() ); }
        std::vector<Point> getCorners()         { return corners; }
        std::vector<PointObj<T>>& getInnerPoints()    { return pts; }
        
        Point ll, lr, ul, ur;
        double side;
        std::vector<Point> corners;
        std::vector<PointObj<T>> pts;
    };

    // splits a single cell into multiple subcells
    void splitCellsAdaptive( std::vector<pCell>& cells, pCell cellTarget, const double& lam );

}

#endif /* cell_hpp */
