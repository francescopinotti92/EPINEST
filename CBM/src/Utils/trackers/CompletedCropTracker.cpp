//
//  CompletedCropTracker.cpp
//  CBM
//
//

#include "CompletedCropTracker.hpp"

EpiStatsCropTracker::EpiStatsCropTracker( NamedOfstream outFile ): BaseCompletedCropTracker( outFile ) {
    
    heading = "farmStrID,d0,d1,t0,t1,size,sizeInf,sizeDead,sizeRemoved\n" ;
    
    // write column names
    if ( !outFile.isClosed() )
        outFile.getStream() << heading ;
    
};

EpiStatsCropTracker::~EpiStatsCropTracker() {};

void EpiStatsCropTracker::reset( bool updateStream ) {

    if ( updateStream ) {
        outFile.update();
        
        // write column names
        if ( !outFile.isClosed() )
            outFile.getStream() << heading ;
        
    }
    else
        outFile.close();

}

void EpiStatsCropTracker::processCompletedCrop( NodeFarm* nf, Crop* crop, const Clock& clock ) {
    
    this->processCompletedCrop( nf->getContext(), crop, clock );
    
}

void EpiStatsCropTracker::processCompletedCrop( Farm* farm, Crop* crop, const Clock& clock ) {
    
    std::string farmID  = farm->getStringID();
    uint dayCreate      = crop->dayCreate;
    uint dayDestroy     = clock.getCurrDay();
    uint tCreate        = crop->tCreate;
    uint tDestroy       = clock.getCurrTime();
    uint size           = crop->nBirdsInitial;
    uint sizeInf        = crop->nInfectionsCumulative;
    uint sizeDead       = crop->nBirdsDead;
    uint sizeRemoved    = crop->nBirdsForceRemove;

    outFile.getStream() << fmt::format( "{},{},{},{},{},{},{},{},{}\n", farmID, dayCreate, dayDestroy, tCreate, tDestroy, size, sizeInf, sizeDead, sizeRemoved ) ;
    
}
