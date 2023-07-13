//
//  warnings.hpp
//  CBM_ECS
//
//

#ifndef warnings_h
#define warnings_h

#include <nlohmann/json.hpp>
#include <iostream>
#include <exception>

// general-purpose warning
struct Warning : public std::exception
{
public:
     Warning(const std::string& msg) {}
     const char* what() { return msg.c_str(); } //message of warning
private:
     std::string msg;
};

struct WrongBirdTypeException : public std::exception {
    std::string msg;
    WrongBirdTypeException(std::string birdTypeName) {
        msg = std::string("unknown bird type: ") + birdTypeName;
    }

    const char * what () const throw () {
       return msg.c_str();
    }
};

struct WrongNodeTypeException : public std::exception {
    std::string msg;
    WrongNodeTypeException(std::string nodeTypeName) {
        msg = std::string("unknown node type: ") + nodeTypeName;
    }

    const char * what () const throw () {
       return msg.c_str();
    }
};

struct WrongActorTypeException : public std::exception {
    std::string msg;
    WrongActorTypeException(std::string actorTypeName) {
        msg = std::string("unknown actor type: ") + actorTypeName;
    }

    const char * what () const throw () {
       return msg.c_str();
    }
};

struct MissingJsonKeyException : public std::exception {
    std::string msg;
    MissingJsonKeyException( std::string key ) {
        msg = std::string("key '") + key + "' not found";
    }

    const char * what () const throw () {
       return msg.c_str();
    }
};

#endif /* warnings_h */
