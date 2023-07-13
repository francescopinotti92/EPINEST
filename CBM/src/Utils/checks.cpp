//
//  checks.cpp
//  CBM
//
//

#include "checks.hpp"

void requireJsonKey( const nlohmann::json& js, const std::string& key ) {
    
    if ( js.count( key ) == 0 ) {
        
        throw MissingJsonKeyException( key );
        
    }
    
}
