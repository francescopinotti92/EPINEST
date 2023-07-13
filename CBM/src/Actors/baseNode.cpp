//
//  baseNode.cpp
//  CBM
//
//

#include "baseNode.hpp"


BaseNode::BaseNode(): pos() {};

void BaseNode::setPos( Geom::Point pos_ ) {
    pos = pos_;
}

Geom::Point BaseNode::getPos() {
    return pos;
}
