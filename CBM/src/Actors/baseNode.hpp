//
//  baseNode.hpp
//  CBM
//
//

#ifndef baseNode_h
#define baseNode_h

//====== Node ======//

#include "geometry_utils.hpp"
#include "point.hpp"
#include "types.hpp"
#include "constants.hpp"
#include "holdsbirds.hpp"
#include "ChickenPool.hpp"

struct Position;

class BaseNode {
public:
    BaseNode();
    virtual ~BaseNode() = default;
    virtual NodeType getNodeType() = 0;
    virtual HoldsBirds* getUnderlyingSetting() = 0;
    virtual void reset( ChickenPool* birdMngr ) = 0;
    virtual uint getSizeBirds() = 0;
    virtual uint getSizeInfectedBirds() = 0;
    virtual double getBaseWeight() = 0;
    virtual void setBaseWeight( double weight ) = 0;
    void setPos( const Geom::Point pos_);
    Geom::Point getPos();
protected:
    Geom::Point pos;
};

#endif /* baseNode_h */
