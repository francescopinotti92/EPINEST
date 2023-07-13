//
//  BaseCompletedCropTracker.hpp
//  CBM
//
//

#ifndef BaseCompletedCropTracker_hpp
#define BaseCompletedCropTracker_hpp

#include "named_ofstream.hpp"
#include "clock.hpp"
#include "bird.hpp"
#include "crop.hpp"
#include "node.hpp"
#include "farm.hpp"
#include "types.hpp"
#include "useful_functions.hpp"

class NodeFarm;

class BaseCompletedCropTracker {
public:
    BaseCompletedCropTracker( NamedOfstream outFile ): outFile( outFile ) {};
    virtual ~BaseCompletedCropTracker() = default;
    virtual void processCompletedCrop( NodeFarm* nf, Crop* crop, const Clock& clock ) = 0;
    virtual void processCompletedCrop( Farm* nf, Crop* crop, const Clock& clock ) = 0;
    virtual void reset( bool updateStream = true ) = 0;
protected:
    NamedOfstream outFile;
};

#endif /* BaseCompletedCropTracker_hpp */
