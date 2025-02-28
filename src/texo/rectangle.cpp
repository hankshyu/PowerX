//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        02/24/2025 20:17:37
//  Module Name:        rectangle.cpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A len_t data type implementation of Rectangle Concept
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//  02/24/2025:         First implementation of rectangle concept in Boost Library using 
//                      self constructed class. Full implementation of documented members.
//
//////////////////////////////////////////////////////////////////////////////////

// Dependencies
// 1. C++ STL:

// 2. Boost Library:
#include "boost/functional/hash.hpp"

// 3. Texo Library:
#include "rectangle.hpp"
#include "units.hpp"
#include "cord.hpp"
#include "fcord.hpp"


std::ostream &operator<<(std::ostream &os, const Rectangle &rec) {

    os << "R[" << boost::polygon::ll(rec);
    os << " W=" << boost::polygon::delta(boost::polygon::horizontal(rec));
    os << " H=" << boost::polygon::delta(boost::polygon::vertical(rec));
    os << " " << boost::polygon::ur(rec) <<"]";

    return os;
}
size_t std::hash<Rectangle>::operator()(const Rectangle &key) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, key.get(boost::polygon::orientation_2d_enum::HORIZONTAL));
    boost::hash_combine(seed, key.get(boost::polygon::orientation_2d_enum::VERTICAL));
    return seed;
}

size_t boost::hash<Rectangle>::operator()(const Rectangle &key) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, key.get(boost::polygon::orientation_2d_enum::HORIZONTAL));
    boost::hash_combine(seed, key.get(boost::polygon::orientation_2d_enum::VERTICAL));
    return seed;
}
