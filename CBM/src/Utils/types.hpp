//
//  types.hpp
//  CBM_ECS
//
//

#ifndef types_h
#define types_h

#include "ElementPool.hpp"
#include <cstdint>
#include <limits>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <memory>
#include <random>
#include <unordered_set>
#include <unordered_map>
#include <stdint.h>

#define FMT_HEADER_ONLY
#include "format.h"
#include "warnings.hpp"

struct Bird;
class infected_set;


//typedef unsigned int uint;
typedef std::uint32_t uint;

//typedef unsigned int    BirdIdType;
typedef unsigned short  StrainIdType; // can not have more than 256 strains
typedef uint64_t  StateType;

typedef std::string             mystring; // change it later in case you want a different string type
typedef std::shared_ptr<Bird>   pBird;
typedef std::weak_ptr<Bird>     wpBird;

typedef std::vector<pBird>          BirdBatch; // This is used in network
//typedef ElementPool<uint>           Crop;

typedef short UnderlyingBirdType;
enum class BirdType: UnderlyingBirdType {   null = 0,
                                            broiler = 1,
                                            deshi = 2 }; // implement birdtype as an enum

typedef short UnderlyingNodeType;
enum class NodeType: UnderlyingNodeType {
    farm = 0,
    middleman = 1,
    market = 2,
    vendor = 3,
    fieldexperimenter = 4
};

const std::vector<NodeType> allNodeTypes { NodeType::farm, NodeType::middleman, NodeType::market, NodeType::vendor };

typedef short UnderlyingActorType;
enum class ActorType: UnderlyingActorType {
    commercial_farm = 0,
    market = 1,
    retailer = 2,
    wholesaler = 3
};

#endif /* types_h */
