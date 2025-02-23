//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        02/23/2025 13:57:53
//  Module Name:        isotropy.cpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        extends Isotropy in Boost libray for expected output
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//
//////////////////////////////////////////////////////////////////////////////////
// Dependencies
// 1. C++ STL:
#include <ostream>
// 2. Boost Library:

// 3. Texo Library:
#include "isotropy.hpp"

std::ostream &operator << (std::ostream &os, const boost::polygon::direction_1d &d){
    if(d == boost::polygon::direction_1d_enum::LOW) os << "Direction1D::LOW";
    else return os << "Direction1D::HIGH";

    return os;
}

std::ostream &operator << (std::ostream &os, const boost::polygon::direction_1d_enum &d){
    if(d == boost::polygon::direction_1d_enum::LOW) os << "Direction1D::LOW";
    else return os << "Direction1D::HIGH";

    return os;
}

std::ostream &operator << (std::ostream &os, const boost::polygon::orientation_2d &orient){
    if(orient == boost::polygon::orientation_2d_enum::HORIZONTAL) os << "Orientation2D::HORIZONTAL";
    else return os << "Orientation2D::VERTICAL";

    return os;
}

std::ostream &operator << (std::ostream &os, const boost::polygon::orientation_2d_enum &orient){
    if(orient == boost::polygon::orientation_2d_enum::HORIZONTAL) os << "Orientation2D::HORIZONTAL";
    else return os << "Orientation2D::VERTICAL";

    return os;
}

std::ostream &operator << (std::ostream &os, const boost::polygon::direction_2d &d){
    switch (d.to_int()){
        case boost::polygon::direction_2d_enum::WEST:
            os << "Direction2D::WEST";
            break;
        case boost::polygon::direction_2d_enum::EAST:
            os << "Direction2D::EAST";
            break;
        case boost::polygon::direction_2d_enum::SOUTH:
            os << "Direction2D::SOUTH";
            break;
        case boost::polygon::direction_2d_enum::NORTH:
            os << "Direction2D::NORTH";
            break;
        default:
            break;
    }

    return os;
}

std::ostream &operator << (std::ostream &os, const boost::polygon::direction_2d_enum &d){
    switch (d){
        case boost::polygon::direction_2d_enum::WEST:
            os << "Direction2D::WEST";
            break;
        case boost::polygon::direction_2d_enum::EAST:
            os << "Direction2D::EAST";
            break;
        case boost::polygon::direction_2d_enum::SOUTH:
            os << "Direction2D::SOUTH";
            break;
        case boost::polygon::direction_2d_enum::NORTH:
            os << "Direction2D::NORTH";
            break;
        default:
            break;
    }

    return os;
}

std::ostream &operator << (std::ostream &os, const boost::polygon::orientation_3d &orient){
    switch (orient.to_int()){
        case boost::polygon::orientation_2d_enum::HORIZONTAL:
            os << "Orientation3D::HORIZONTAL";
            break;
        case boost::polygon::orientation_2d_enum::VERTICAL:
            os << "Orientation3D::VERTICAL";
            break;
            case boost::polygon::orientation_3d_enum::PROXIMAL:
            os << "Orientation3D::PROXIMAL";
            break;
        default:
            break;
    }
    
    return os;
}

std::ostream &operator << (std::ostream &os, const boost::polygon::orientation_3d_enum &orient){
    
    if(orient == boost::polygon::orientation_3d_enum::PROXIMAL) return os << "Orientation3D::PROXIMAL";
    else return os;
}

std::ostream &operator << (std::ostream &os, const boost::polygon::direction_3d &d){
    switch (d.to_int()){
        case boost::polygon::direction_2d_enum::WEST:
            os << "Direction3D::WEST";
            break;
        case boost::polygon::direction_2d_enum::EAST:
            os << "Direction3D::EAST";
            break;
        case boost::polygon::direction_2d_enum::SOUTH:
            os << "Direction3D::SOUTH";
            break;
        case boost::polygon::direction_2d_enum::NORTH:
            os << "Direction3D::NORTH";
            break;
        case boost::polygon::direction_3d_enum::DOWN:
            os << "Direction3D::DOWN";
            break;
        case boost::polygon::direction_3d_enum::UP:
            os << "Direction3D::UP";
            break;
        default:
            break;
    }
    
    return os;
}

std::ostream &operator << (std::ostream &os, const boost::polygon::direction_3d_enum &d){
    switch (d){
        case boost::polygon::direction_3d_enum::DOWN:
            os << "Direction3D::DOWN";
            break;
        case boost::polygon::direction_3d_enum::UP:
            os << "Direction3D::UP";
            break;
        default:
            break;
    }
    
    return os;
}
