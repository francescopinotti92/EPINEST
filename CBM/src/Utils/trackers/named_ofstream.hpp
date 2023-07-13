//
//  named_ofstream.hpp
//  CBM
//
//

#ifndef named_ofstream_hpp
#define named_ofstream_hpp

#include <fstream>
#include "types.hpp"

// wraps std::ofstream and adds functionalities:
// stream can be closed and re-opened between consecutive simulations
// n.b. each NamedOfstream should be associated to a single type of analysis
class NamedOfstream {
public:
    NamedOfstream();
    NamedOfstream( std::string path, std::string baseName, std::string extension, std::string scenarioID, uint simulationID = 0 );
    std::ofstream& getStream();
    void update();
    void update( uint newSimID );
    void close();
    bool isClosed();
private:
    std::string path;
    std::string baseName;
    std::string extension;
    std::string fullPath;
    std::string scenarioID;
    uint simulationID;
    std::ofstream* outFile;
    bool isOpenFlag;
    bool isInertFlag;
};

#endif /* named_ofstream_hpp */
