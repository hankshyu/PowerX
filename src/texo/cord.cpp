//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        02/21/2025 23:19:18
//  Module Name:        cord.cpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A len_t data type of boost::point_concept implementation
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//  02/21/2025:         Reimplement Cord member functions, using rule of zero
//
//////////////////////////////////////////////////////////////////////////////////

// Dependencies
// 1. C++ STL:

// 2. Boost Library:
#include "boost/functional/hash.hpp"

// 3. Texo Library:
#include "cord.hpp"


Cord::Cord() = default;

Cord::Cord(len_t x, len_t y): m_x(x), m_y(y){

}

Cord::operator FCord() const {
    return FCord(static_cast<flen_t>(m_x), static_cast<flen_t>(m_y));
}

bool Cord::operator==(const Cord &other) const{
    return (m_x == other.m_x) && (m_y == other.m_y); 
}

bool Cord::operator!=(const Cord &other) const{
    return (m_x != other.m_x) || (m_y != other.m_y);
}

bool Cord::operator<(const Cord &other) const{
    return (m_y != other.m_y)? (m_y < other.m_y) : (m_x < other.m_x);
}

bool Cord::operator<=(const Cord &other) const{
    return (m_y != other.m_y) ? (m_y < other.m_y) : (m_x <= other.m_x);
}

bool Cord::operator>(const Cord &other) const{
    return (m_y != other.m_y)? (m_y > other.m_y) : (m_x > other.m_x);
}
 
bool Cord::operator>=(const Cord &other) const{
    return (m_y != other.m_y) ? (m_y > other.m_y) : (m_x >= other.m_x);
}

len_t Cord::get(boost::polygon::orientation_2d orient) const{
    return (orient == boost::polygon::orientation_2d_enum::HORIZONTAL) ? m_x : m_y;
}

len_t Cord::x() const{
    return m_x;
}
len_t Cord::y() const{
    return m_y;
}

void Cord::set(boost::polygon::orientation_2d orient, len_t value){
    if(orient == boost::polygon::orientation_2d_enum::HORIZONTAL) m_x = value;
    else m_y = value;
}

void Cord::x(len_t value){
    m_x = value;
}

void Cord::y(len_t value){
    m_y = value;
}

void swap(Cord &first, Cord &second) noexcept{
    std::swap(first.m_x, second.m_x);
    std::swap(first.m_y, second.m_y);
}

std::ostream &operator<<(std::ostream &os, const Cord &c) {
    return os << "(" << c.m_x << ", " << c.m_y << ")";
}

size_t std::hash<Cord>::operator()(const Cord &key) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, key.x());
    boost::hash_combine(seed, key.y());
    return seed;
}

size_t boost::hash<Cord>::operator()(const Cord &key) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, key.x());
    boost::hash_combine(seed, key.y());
    return seed;
}

len_t calL1Distance(const Cord &c1, const Cord &c2) noexcept {
    return boost::polygon::manhattan_distance<Cord, Cord>(c1, c2);
}

flen_t calL2Distance(const Cord &c1, const Cord &c2) noexcept{
    return boost::polygon::euclidean_distance<Cord, Cord>(c1, c2);
}

len_t calDistanceSquared(const Cord &c1, const Cord &c2) noexcept{
    return boost::polygon::distance_squared<Cord, Cord>(c1, c2);
}






