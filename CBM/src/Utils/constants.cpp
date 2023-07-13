//
//  constants.cpp
//  CBM
//
//

#include "constants.hpp"


namespace constants {

std::unordered_map<BirdType, const uint> harvestTimes = { { BirdType::broiler, 32 }, { BirdType::deshi, 40 } };

BirdType getBirdTypeFromString( const std::string& birdTypeString ) {
    if ( birdTypeString == "broiler" or birdTypeString == "BR" )
        return BirdType::broiler;
    else if ( birdTypeString == "deshi" )
        return BirdType::deshi;
    else
        throw WrongBirdTypeException(birdTypeString);
}

std::string getStringFromBirdType( const BirdType& bt ) {
    if ( bt == BirdType::broiler )
        return "broiler";
    else
        return "none";
}

NodeType getNodeTypeFromString( const std::string& nodeTypeString ) {
    
    if ( ( nodeTypeString == "Farm" ) or ( nodeTypeString == "farm" ) ) {
        return NodeType::farm;
    }
    else if ( ( nodeTypeString == "Middleman" ) or ( nodeTypeString == "middleman" ) ) {
        return NodeType::middleman;
    }
    else if ( ( nodeTypeString == "Market" ) or ( nodeTypeString == "market" ) ) {
        return NodeType::market;
    }
    else if ( ( nodeTypeString == "Vendor" ) or ( nodeTypeString == "vendor" ) ) {
        return NodeType::vendor;
    }
    else
        throw WrongNodeTypeException( nodeTypeString );
        
}

ActorType getActorTypeFromString( const std::string& actorTypeString ) {
    if ( actorTypeString == "CommercialFarm" )
        return ActorType::commercial_farm;
    else if (actorTypeString == "Market")
        return ActorType::market;
    else
        throw WrongActorTypeException( actorTypeString );
}

std::string getStringFromNodeType( const NodeType& nt ) {
    switch ( nt ) {
        case NodeType::farm:
            return "F";
        case NodeType::market:
            return "M";
        case NodeType::vendor:
            return "V";
        case NodeType::middleman:
            return "MM";
        case NodeType::fieldexperimenter:
            return "FE";
    }
}

}
