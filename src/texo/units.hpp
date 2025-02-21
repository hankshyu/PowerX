//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        02/21/2025 10:53:12 PM
//  Module Name:        units.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//  Dependencies
//  1. C++ STL:         cstdint, climits, ostream
//  2. Boost Library:   boost/polygon/polygon.hpp
//  3. Texo Library:    None
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        Defines the basic datatypes and some global definitions
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//  02/21/2025:         Remove sector struct and some redundant functions
//////////////////////////////////////////////////////////////////////////////////

#ifndef __UNITS_H__
#define __UNITS_H__

// Dependencies
// 1. C++ STL:
#include <cstdint>
#include <climits>
#include <ostream>

// 2. Boost Library:
#include "boost/polygon/polygon.hpp"

// 3. Texo Library:

typedef int32_t len_t;
typedef int64_t  area_t;

typedef double flen_t;
typedef double angle_t;

// enum direction_1d_enum {LOW = 0, HIGH = 1,
//                         LEFT = 0, RIGHT = 1,
//                         CLOCKWISE = 0, COUNTERCLOCKWISE = 1,
//                         REVERSE = 0, FORWARD = 1,
//                         NEGATIVE = 0, POSITIVE = 1 };
typedef boost::polygon::direction_1d_enum direction1D;
std::ostream &operator << (std::ostream &os, const direction1D &d);

// enum orientation_2d_enum { HORIZONTAL = 0, VERTICAL = 1 };
typedef boost::polygon::orientation_2d_enum orientation2D;
std::ostream &operator << (std::ostream &os, const orientation2D &o);

// enum direction_2d_enum { WEST = 0, EAST = 1, SOUTH = 2, NORTH = 3 };
enum class direction2D{
    NORTH = 0, EAST = 1, SOUTH = 2, WEST = 3,
    NORTHEAST = 4, SOUTHEAST = 5, SOUTHWEST = 6, NORTHWEST = 7,
    CENTRE = 8,
    
    UP = 0, RIGHT = 1, DOWN = 2, LEFT = 3
};
std::ostream &operator << (std::ostream &os, const direction2D &o);

enum class quadrant{
    I, II, III, IV
};
std::ostream &operator << (std::ostream &os, const quadrant &q);


#define LEN_T_MAX std::numeric_limits<len_t>::max()
#define LEN_T_MIN std::numeric_limits<len_t>::min()

#define FLEN_T_MAX std::numeric_limits<flen_t>::max()
#define FLEN_T_MIN std::numeric_limits<flen_t>::min()

#define AREA_T_MAX std::numeric_limits<area_t>::max()
#define AREA_T_MIN std::numeric_limits<area_t>::min()

angle_t flipAngle(angle_t angle);
quadrant translateAngleToQuadrant(angle_t angle);



#endif // __UNITS_H__