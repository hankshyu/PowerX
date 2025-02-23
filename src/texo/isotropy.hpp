//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        02/23/2025 13:57:53
//  Module Name:        isotropy.hpp
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

#ifndef __ISOTROPY_H__
#define __ISOTROPY_H__
// Dependencies
// 1. C++ STL:
#include <ostream>

// 2. Boost Library:
#include "boost/polygon/polygon.hpp"

// 3. Texo Library:


// enum direction_1d_enum {LOW = 0, HIGH = 1,
//                         LEFT = 0, RIGHT = 1,
//                         CLOCKWISE = 0, COUNTERCLOCKWISE = 1,
//                         REVERSE = 0, FORWARD = 1,
//                         NEGATIVE = 0, POSITIVE = 1 };

typedef boost::polygon::direction_1d Direction1D;
typedef boost::polygon::direction_1d_enum eDirection1D;

std::ostream &operator << (std::ostream &os, const boost::polygon::direction_1d &d);
std::ostream &operator << (std::ostream &os, const boost::polygon::direction_1d_enum &d);



// enum orientation_2d_enum { HORIZONTAL = 0, VERTICAL = 1 };

typedef boost::polygon::orientation_2d Orientation2D;
typedef boost::polygon::orientation_2d_enum eOrientation2D;

std::ostream &operator << (std::ostream &os, const boost::polygon::orientation_2d &orient);
std::ostream &operator << (std::ostream &os, const boost::polygon::orientation_2d_enum &orient);



// enum direction_2d_enum { WEST = 0, EAST = 1, SOUTH = 2, NORTH = 3 };

typedef boost::polygon::direction_2d Direction2D;
typedef boost::polygon::direction_2d_enum eDirection2D;

std::ostream &operator << (std::ostream &os, const boost::polygon::direction_2d &d);
std::ostream &operator << (std::ostream &os, const boost::polygon::direction_2d_enum &d);



// enum orientation_3d_enum { HORIZONTAL = 0, VERTICAL = 1, PROXIMAL = 2 };

typedef boost::polygon::orientation_3d Orientation3D;
typedef boost::polygon::orientation_3d_enum eOrientation3D;

std::ostream &operator << (std::ostream &os, const boost::polygon::orientation_3d &orient);
std::ostream &operator << (std::ostream &os, const boost::polygon::orientation_3d_enum &orient);



// enum direction_3d_enum { WEST = 0, EAST = 1, SOUTH = 2, NORTH = 3, DOWN = 4, UP = 5 };

typedef boost::polygon::direction_3d Direction3D;
typedef boost::polygon::direction_3d_enum eDirection3D;

std::ostream &operator << (std::ostream &os, const boost::polygon::direction_3d &d);
std::ostream &operator << (std::ostream &os, const boost::polygon::direction_3d_enum &d);

#endif // __ISOTROPY_H__