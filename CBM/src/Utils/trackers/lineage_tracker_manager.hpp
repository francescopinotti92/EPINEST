//
//  lineage_tracker_manager.hpp
//  CBM
//
//

#ifndef lineage_tracker_manager_hpp
#define lineage_tracker_manager_hpp

#include "LineageTree.hpp"
#include "holdsbirds.hpp"
#include "bird.hpp"
#include "types.hpp"
#include "clock.hpp"
#include <vector>
#include <stdio.h>


class LineageTrackerManager {
public:
    LineageTrackerManager( const uint& S, const uint& maxStrains ) ;
    void reset() ;
    void addInfectionEvent( const Bird& infectee, const Bird& infector, const StrainIdType& strain, const Clock& clock ) ;
    void addExternalInfectionEvent( const Bird& infectee, const StrainIdType& strain, const Clock& clock ) ;
    void addRecoveryEvent( const Bird& bird, const StrainIdType& strain ) ;
    void addRemovalEvent( const Bird& bird ) ;
    uint getStrainNumber();
    LineageTree<BirdID, DataLineage, hashBirdID>* getTree( StrainIdType& strain ) ;
    std::vector<StrainIdType>& getStrainList( const StateType& state );
    
private:
    uint S ; // number of strains
    std::vector<LineageTree<BirdID, DataLineage, hashBirdID>*> trees;
    std::unordered_map<StateType, std::vector<StrainIdType>> strainList;

} ;


#endif /* lineage_tracker_manager_hpp */
