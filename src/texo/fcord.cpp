//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        02/22/2025 22:53:53 PM
//  Module Name:        fcord.cpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A flen_t data type of boost::point_concept implementation
//                      Note that boost::polygon library doest not well support
//                      floating point type coordinate systems in most data structures
//                      This implementation merely serves for calculation purposes
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//  02/21/2025:         Reimplement FFCord member functions, using rule of zero
//
//////////////////////////////////////////////////////////////////////////////////

// Dependencies
// 1. C++ STL:

// 2. Boost Library:
#include "boost/functional/hash.hpp"

// 3. Texo Library:
#include "fcord.hpp"


FCord::FCord() = default;

FCord::FCord(flen_t x, flen_t y): m_x(x), m_y(y){

};

bool FCord::operator==(const FCord &other) const{
    return (m_x == other.m_x) && (m_y == other.m_y); 
}

bool FCord::operator!=(const FCord &other) const{
    return (m_x != other.m_x) || (m_y != other.m_y);
}

bool FCord::operator<(const FCord &other) const{
    return (m_y != other.m_y)? (m_y < other.m_y) : (m_x < other.m_x);
}

bool FCord::operator<=(const FCord &other) const{
    return (m_y != other.m_y) ? (m_y < other.m_y) : (m_x <= other.m_x);
}

bool FCord::operator>(const FCord &other) const{
    return (m_y != other.m_y)? (m_y > other.m_y) : (m_x > other.m_x);
}
 
bool FCord::operator>=(const FCord &other) const{
    return (m_y != other.m_y) ? (m_y > other.m_y) : (m_x >= other.m_x);
}

flen_t FCord::get(boost::polygon::orientation_2d orient) const{
    return (orient == boost::polygon::orientation_2d_enum::HORIZONTAL) ? m_x : m_y;
}

flen_t FCord::x() const{
    return m_x;
}
flen_t FCord::y() const{
    return m_y;
}

void FCord::set(boost::polygon::orientation_2d orient, flen_t value){
    if(orient == boost::polygon::orientation_2d_enum::HORIZONTAL) m_x = value;
    else m_y = value;
}

void FCord::x(flen_t value){
    m_x = value;
}

void FCord::y(flen_t value){
    m_y = value;
}

void swap(FCord &first, FCord &second) noexcept{
    std::swap(first.m_x, second.m_x);
    std::swap(first.m_y, second.m_y);
}

std::ostream &operator<<(std::ostream &os, const FCord &c) {
    return os << "(" << c.m_x << ", " << c.m_y << ")";
}

size_t std::hash<FCord>::operator()(const FCord &key) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, key.x());
    boost::hash_combine(seed, key.y());
    return seed;
}

flen_t calL1Distance(const FCord &c1, const FCord &c2) noexcept {
    return boost::polygon::manhattan_distance<FCord, FCord>(c1, c2);
}

flen_t calL2Distance(const FCord &c1, const FCord &c2) noexcept{
    return boost::polygon::euclidean_distance<FCord, FCord>(c1, c2);
}

flen_t calDistanceSquared(const FCord &c1, const FCord &c2) noexcept{
    return boost::polygon::distance_squared<FCord, FCord>(c1, c2);
}
