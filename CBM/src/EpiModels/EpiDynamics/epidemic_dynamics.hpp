//
//  epidemic_dynamics.hpp
//  CBM
//
//

#ifndef epidemic_dynamics_hpp
#define epidemic_dynamics_hpp

#include "basecompartmentalmodel.hpp"
#include "infection_tracer.hpp"
#include "holdsbirds.hpp"
#include "random.hpp"

// homogeneous-mixing transmission within single setting
void runBirdTransmissionSingleSetting( HoldsBirds& setting, const Clock& clock, BaseCompartmentalModel& compModel, Random& randGen );

void runBirdTransmissionSingleSettingWithTracking( HoldsBirds& setting, const Clock& clock, BaseCompartmentalModel& compModel, Random& randGen, BaseInfectionTracker* tracker );

// environmental transmission within single setting
void runEnvTransmissionSingleSetting( HoldsBirds& setting, const Clock& clock, BaseCompartmentalModel& compModel, Random& randGen );

void runEnvTransmissionSingleSettingWithTracking( HoldsBirds& setting, const Clock& clock, BaseCompartmentalModel& compModel, Random& randGen, BaseInfectionTracker* tracker );

// transmission between two settings
void runTransmissionTwoSettings(HoldsBirds& setting1, HoldsBirds& setting2, const uint& nContacts, const Clock& clock, BaseCompartmentalModel& compModel, Random& randGen );

void runTransmissionTwoSettingsWithTracking(HoldsBirds& setting1, HoldsBirds& setting2, const uint& nContacts, const Clock& clock, BaseCompartmentalModel& compModel, Random& randGen, BaseInfectionTracker* tracker);

void sparkInfectionTwoSettings( HoldsBirds& setting1, HoldsBirds& setting2, const Clock& clock, BaseCompartmentalModel& compModel, Random& randGen );

void sparkInfectionTwoSettingsWithTracking( HoldsBirds& setting1, HoldsBirds& setting2, const Clock& clock, BaseCompartmentalModel& compModel, Random& randGen, BaseInfectionTracker* tracker );

void sparkInfectionSingleSetting( HoldsBirds& setting, const StrainIdType& strain, const double& sparkRate, const Clock& clock, BaseCompartmentalModel& compModel, Random& randGen );

void sparkInfectionSingleSettingWithTracking( HoldsBirds& setting, const StrainIdType& strain, const double& sparkRate, const Clock& clock, BaseCompartmentalModel& compModel, Random& randGen, BaseInfectionTracker* tracker );

void forceSparkInfectionSingleSetting( HoldsBirds& setting, const StrainIdType& strain, const uint& nInfections, const Clock& clock, BaseCompartmentalModel& compModel, Random& randGen );

void forceSparkInfectionSingleSettingWithTracking( HoldsBirds& setting, const StrainIdType& strain, const uint& nInfections, const Clock& clock, BaseCompartmentalModel& compModel, Random& randGen, BaseInfectionTracker* tracker );

// other stuff

void applyDirectInfection(pBird& bird, const StrainIdType& strain, const uint& tCurr, EventPriorityQueue<BirdStatusEvent>& recovEventQueue, const BaseCompartmentalModel& compModel, Random& randGen);

#endif /* epidemic_dynamics_hpp */
