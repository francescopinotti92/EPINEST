//
//  CompletedCropTracker.hpp
//  CBM
//
//

#ifndef CompletedCropTracker_hpp
#define CompletedCropTracker_hpp

#include <stdio.h>
#include "BaseCompletedCropTracker.hpp"
#include "ElementPool.hpp"

//=== EpiStatsCropTracker ===//

class EpiStatsCropTracker: public BaseCompletedCropTracker {
public:
    EpiStatsCropTracker( NamedOfstream outFile );
    ~EpiStatsCropTracker() override;
    void processCompletedCrop( NodeFarm* nf, Crop* crop, const Clock& clock ) override;
    void processCompletedCrop( Farm* nf, Crop* crop, const Clock& clock ) override;

    void reset( bool updateStream = true ) override;
private:
    std::string heading;
    
};

#endif /* CompletedCropTracker_hpp */
