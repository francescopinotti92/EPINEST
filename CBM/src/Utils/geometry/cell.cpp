//
//  cell.cpp
//  CBM
//
//

#include "cell.hpp"


namespace Geom {

    //===== Cell =====///

    Cell::Cell(): side( 0. ), ll(), lr(), ul(), ur() {
        
        corners = { ll, lr, ul, ur };
        pts = {};
    }

    Cell::Cell( const double& lb_x, const double& lb_y, const double& side ): side( side ) {
        
        ll = Point( lb_x, lb_y );
        lr = Point( lb_x + side, lb_y );
        ul = Point( lb_x, lb_y + side );
        ur = Point( lb_x + side, lb_y + side );
        
        corners = { ll, lr, ul, ur };
        
        pts = {};
        
    }
    
    bool Cell::contains_point( const Point& p ) {
        
        if ( ( p.getX() < ll.getX() ) or ( p.getX() > lr.getX() ) )
            return false;
        
        if ( ( p.getY() < ll.getY() ) or ( p.getY() > ul.getY() ) )
            return false;
        
        return true;
        
    }


    bool Cell::isWithinHRange( const Point& p ) {
        
        if( p.getX() >= this->ll.getX() && p.getX() <= this->lr.getX() )
            return true;
        else
            return false;
        
    }

    bool Cell::isWithinVRange( const Point& p ) {
        
        if( p.getY() >= this->ll.getY() && p.getY() <= this->ul.getY() )
            return true;
        else
            return false;
    }

    void Cell::addPoint( const Point& pt ) {
        
        pts.push_back( pt );
        
    }

    void Cell::addPoints( const std::vector<Point>& pts_ ) {
        
        pts.reserve( pts_.size() );
        
        for ( const Point& pt: pts_ )
            addPoint( pt );
    }


    //Calculates the shortest distance to another cell inside a grid with heterogeneously sized cells as created by the adaptive gridding method.

    double Cell::computeDistanceFromCell( Cell other_cell ) {
        
        bool shortest_distance_found = false;
        std::vector<double> new_distances;
        new_distances.reserve(8);

        std::vector<Point> other_corners = other_cell.getCorners();

        for( Point other_p : other_corners ) {
            for( Point own_p : corners ) {
                //if c1 & c2 share a point they are adjacent and d = 0
                if( own_p == other_p ) {
                    new_distances.push_back( 0. );
                    shortest_distance_found = true;
                    break;
                }
            }
        }

        //If an edge of c2 overlaps with an edge in c1, the two points
        //between which the shortest distance is found is on a straight horizontal or
        //vertical line (in which case we don't need to use the distance function).
        //Check for horizontal alignment first.
        if( !shortest_distance_found )
        {
            for( Point other_p : other_corners )
            {
                if( this->isWithinHRange( other_p ) )
                {
                    new_distances.push_back( std::abs(this->ll.getY() - other_p.getY() ) );
                    new_distances.push_back( std::abs(this->ul.getY() - other_p.getY() ) );
                    shortest_distance_found = true;
                }
            }
            for( Point own_p : this->corners )
            {
                if( other_cell.isWithinHRange( own_p ) )
                {
                    new_distances.push_back(std::abs( other_corners[0].getY() - own_p.getY() ) );
                    new_distances.push_back(std::abs( other_corners[2].getY() - own_p.getY() ) );
                    shortest_distance_found = true;
                }
            }

            //Check for vertical alignment
            for( Point other_p : other_corners )
            {
                if( this->isWithinVRange( other_p ) )
                {
                    new_distances.push_back( std::abs( this->ll.getX() - other_p.getX() ) );
                    new_distances.push_back( std::abs( this->lr.getX() - other_p.getX() ) );
                    shortest_distance_found = true;
                }
            }
            for( Point own_p : this->corners )
            {
                if( other_cell.isWithinVRange( own_p ) )
                {
                    new_distances.push_back( std::abs( other_corners[0].getX() - own_p.getX() ) );
                    new_distances.push_back( std::abs( other_corners[1].getX() - own_p.getX() ) );
                    shortest_distance_found = true;
                }
            }
        }
        //No vertical or horizontal overlap, must use euclidean distance
        if( !shortest_distance_found )
        {
            for( Point own_p : this->corners )
            {
                for( Point other_p : other_corners )
                {
                    new_distances.push_back( euclidean_distance( own_p, other_p ) );
                }
            }
        }

        //Get minimum from the vector and return it.
        return *std::min_element( new_distances.begin(), new_distances.end() );
    }


    void splitCellsAdaptive( std::vector<pCell>& cells, pCell cellTarget, const double& lam ) {
                
        // get info from target cell
        
        int n = cellTarget->get_npoints();
        
        if ( n == 0 )
            return;
            
        double llx  = cellTarget->get_llx();
        double lly  = cellTarget->get_lly();
        double side = cellTarget->get_side();
        std::vector<Point> nodes = cellTarget->getInnerPoints();
        
        std::vector<pCell> children = {};
        children.reserve( 4 );
        
        children.push_back( std::make_shared<Cell>( llx, lly, side * 0.5 ) );
        children.push_back( std::make_shared<Cell>( llx + side * 0.5, lly, side * 0.5 ) );
        children.push_back( std::make_shared<Cell>( llx, lly + side * 0.5, side * 0.5 ) );
        children.push_back( std::make_shared<Cell>( llx + side * 0.5, lly + side * 0.5, side * 0.5 ) );
        

        int n_nonempty_children = 0;
        double log_lam = log( lam );
        double p_var = pow( log( n ) - log_lam, 2. );
        double c_var = 0.;
        
        std::vector<double> log_c_sizes;
        
        for ( auto& child : children ) {
            
            for ( auto& pt : nodes ) {
                
                if ( child->contains_point( pt ) ) {
                    
                    child->addPoint( pt );
                    
                }
                
            }
            
            if ( child->get_npoints() > 0 ) {
                
                c_var += pow( log( child->get_npoints() ) - log_lam, 2. );
                ++n_nonempty_children;
                
            }
            
        }
        
        c_var = c_var / n_nonempty_children;
        
        std::vector<pCell> accepted_children = {};
        accepted_children.reserve( 4 );
        
        if ( p_var > c_var ) {
            
            for ( int i = 0; i < 4; ++i ) {
                
                pCell child = children[i];
                
                if ( !child->isEmpty() ) {
                    
                    accepted_children.push_back( child );
                    cells.push_back( child );
                    
                }
            }
           
            // remove parent cell
            for ( auto it = cells.begin(), end = cells.end(); it < end; ++it ) {
                
                if ( ( *it ) == cellTarget ) {
                    std::iter_swap( it, cells.end() - 1 );
                    //delete cells.back();
                    //delete cellTarget;
                    cells.pop_back();
                    break;
                }
                
            }
            
            // split children recursively
            
            for ( pCell child : accepted_children )
                splitCellsAdaptive( cells, child, lam );
    
        }
        else {
            
            //for ( pCell child : children )
            //    delete child;
            
            return;
            
        }
        
    }


//===== Cell =====///
    
    template <typename T>
    CellObj<T>::CellObj(): side( 0. ), ll(), lr(), ul(), ur() {
        
        corners = { ll, lr, ul, ur };
        pts = {};
    }

    template <typename T>
    CellObj<T>::CellObj( const double& lb_x, const double& lb_y, const double& side ): side( side ) {
        
        ll = Point( lb_x, lb_y );
        lr = Point( lb_x + side, lb_y );
        ul = Point( lb_x, lb_y + side );
        ur = Point( lb_x + side, lb_y + side );
        
        corners = { ll, lr, ul, ur };
        
        pts = {};
        
    }

    template <typename T>
    bool CellObj<T>::contains_point( const Point& p ) {
        
        if ( ( p.getX() < ll.getX() ) or ( p.getX() > lr.getX() ) )
            return false;
        
        if ( ( p.getY() < ll.getY() ) or ( p.getY() > ul.getY() ) )
            return false;
        
        return true;
        
    }

    template <typename T>
    bool CellObj<T>::isWithinHRange( const Point& p ) {
        
        if( p.getX() >= this->ll.getX() && p.getX() <= this->lr.getX() )
            return true;
        else
            return false;
        
    }

    template <typename T>
    bool CellObj<T>::isWithinVRange( const Point& p ) {
        
        if( p.getY() >= this->ll.getY() && p.getY() <= this->ul.getY() )
            return true;
        else
            return false;
    }

    template <typename T>
    template <typename U>
    bool CellObj<T>::contains_point( const PointObj<U>& p ) {
        
        if ( ( p.getX() < ll.getX() ) or ( p.getX() > lr.getX() ) )
            return false;
        
        if ( ( p.getY() < ll.getY() ) or ( p.getY() > ul.getY() ) )
            return false;
        
        return true;
        
    }

    template <typename T>
    template <typename U>
    bool CellObj<T>::isWithinHRange( const PointObj<U>& p ) {
        
        if( p.x() >= this->ll.x && p.x() <= this->lr.x )
            return true;
        else
            return false;
        
    }

    template <typename T>
    template <typename U>
    bool CellObj<T>::isWithinVRange( const PointObj<U>& p ) {
        
        if( p.y() >= this->ll.y && p.y() <= this->ul.x )
            return true;
        else
            return false;
    }

    template <typename T>
    void CellObj<T>::addPoint( const PointObj<T>& pt, const bool& check ) {
        
        if ( check )
            if ( !contains_point( pt ) )
                return;
        
        pts.push_back( pt );
        
    }

    

    
}


/*
 Setting up a grid:
 1) Create a vector of points (only farms)
 2) Create a cell large enough to contain all these points; fill it with points
 3) Call recursive algorithm
 */
