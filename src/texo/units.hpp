//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        02/21/2025 22:53:12
//  Module Name:        units.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        Defines the basic datatypes
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//  02/21/2025:         Remove sector struct and some redundant functions
//  02/23/2025:         Move Isotropy to isotropy.hpp, leaves unit definitions
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
typedef int64_t area_t;

typedef double flen_t;
typedef double farea_t;
typedef double angle_t;

#define LEN_T_MAX std::numeric_limits<len_t>::max()
#define LEN_T_MIN std::numeric_limits<len_t>::min()

#define AREA_T_MAX std::numeric_limits<area_t>::max()
#define AREA_T_MIN std::numeric_limits<area_t>::min()

#define FLEN_T_MAX std::numeric_limits<flen_t>::max()
#define FLEN_T_MIN std::numeric_limits<flen_t>::min()

#define FAREA_T_MAX std::numeric_limits<farea_t>::max()
#define FAREA_T_MIN std::numeric_limits<farea_t>::min()

enum class quadrant{
    I, II, III, IV
};
std::ostream &operator << (std::ostream &os, const quadrant &q);

angle_t flipAngle(angle_t angle);
quadrant translateAngleToQuadrant(angle_t angle);



#endif // __UNITS_H__