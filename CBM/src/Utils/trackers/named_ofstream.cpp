//
//  named_ofstream.cpp
//  CBM
//
//

#include "named_ofstream.hpp"
#include "useful_functions.hpp"

// default constructor
// creates an inert NamedOfStream
NamedOfstream::NamedOfstream(): path(""), baseName(""), extension(""), scenarioID(""), simulationID(0) {
    fullPath = "";
    outFile = nullptr;
    isOpenFlag = false;
    isInertFlag = true;
}

// constructor
// creates and opens file at desired path
NamedOfstream::NamedOfstream( std::string path_, std::string baseName_, std::string extension_, std::string scenarioID_, uint simulationID_ ): path( path_ ), baseName( baseName_ ), extension( extension_ ), scenarioID( scenarioID_ ), simulationID( simulationID_ ) {
    
    // this path formatting will not work on Windows
    fullPath = fmt::format( "{}/{}_{}_{}.{}", path, baseName, scenarioID, simulationID, extension );
    
    assert( does_path_exist( path ) );
    
    outFile = new std::ofstream( fullPath, std::ofstream::out );
    isOpenFlag = true;
    isInertFlag = false;
}

// get the underlying ofstream
// please first check if NamedOfstream is open before
// calling getStream()
std::ofstream& NamedOfstream::getStream() {
    return *outFile;
}

// close ofstream for good
void NamedOfstream::close() {
    
    if ( isInertFlag )
        return;
    
    outFile->close();
    delete outFile;
    isOpenFlag = false;
    isInertFlag = true;
}

// closes ofstream and opens a new one.
// creates a new file with same name except for the index part
void NamedOfstream::update() {
    
    if ( isInertFlag )
        return;
    
    if ( !isClosed() ) {
        outFile->close();
        delete outFile;
    }

    simulationID++;
    fullPath = fmt::format( "{}/{}_{}_{}.{}", path, baseName, scenarioID, simulationID, extension );
    outFile = new std::ofstream( fullPath, std::ofstream::out );
    isOpenFlag = true;
}

// as above, but allows to specify the simulation index
void NamedOfstream::update( uint newSimID ) {
    
    if ( isInertFlag )
        return;
    
    if ( !isClosed() ) {
        outFile->close();
        delete outFile;
    }

    simulationID = newSimID;
    fullPath = fmt::format( "{}/{}_{}_{}.{}", path, baseName, scenarioID, simulationID, extension );
    outFile = new std::ofstream( fullPath, std::ofstream::out );
    isOpenFlag = true;
}

// returns 'true' iff ofstream is closed
bool NamedOfstream::isClosed() {
    return !isOpenFlag;
}
