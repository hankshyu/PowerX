//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        02/23/2025 23:12:09
//  Module Name:        line.cpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A Cord data type of boost::segment_concept implementation
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//
//////////////////////////////////////////////////////////////////////////////////

// Dependencies
// 1. C++ STL:

// 2. Boost Library:
#include "boost/functional/hash.hpp"

// 3. Texo Library:
#include "line.hpp"
#include "cord.hpp"
#include "units.hpp"


Line::Line():m_low(Cord(LEN_T_MIN, LEN_T_MIN)), m_high(Cord(LEN_T_MAX, LEN_T_MAX)){

}

Line::Line(const Cord &low, const Cord &high){
    if(low <= high){
        m_low = low;
        m_high = high;
    }else{
        m_low = high;
        m_high = low;
    }
}

bool Line::operator == (const Line &other) const{
    return (m_low == other.m_low) && (m_high == other.m_high);
}

bool Line::operator!=(const Line &other) const{
    return (m_low != other.m_low) || (m_high != other.m_high);
}

bool Line::operator<(const Line &other) const{
    return (m_low != other.m_low)? (m_low < other.m_low) : (m_high < other.m_high);
}

bool Line::operator<=(const Line &other) const{
    return (m_low != other.m_low)? (m_low < other.m_low) : (m_high <= other.m_high);
}

bool Line::operator>(const Line &other) const{
    return (m_low != other.m_low)? (m_low > other.m_low) : (m_high > other.m_high);
}

bool Line::operator>=(const Line &other) const{
    return (m_low != other.m_low)? (m_low > other.m_low) : (m_high >= other.m_high);
}

Cord Line::get(boost::polygon::direction_1d dir) const{
    return (dir == boost::polygon::direction_1d_enum::LOW)? m_low : m_high;
}

Cord Line::low() const{
    return m_low;
}

Cord Line::high() const{
    return m_high;
}

void Line::set(boost::polygon::direction_1d dir, const Cord &point){
    if(dir == boost::polygon::direction_1d_enum::LOW){
        if(point <= m_high) m_low = point;
        else{
            m_low = m_high;
            m_high = point;
        }
    }else{ // dir == boost::polygon::direction_1d_enum::HIGH
        if(point >= m_low) m_high = point;
        else{
            m_high = m_low;
            m_low = point;
        }
    }
}

Line &Line::low(const Cord &point){
    if(point <= m_high) m_low = point;
    else{
        m_low = m_high;
        m_high = point;
    }

    return *this;
}

Line &Line::high(const Cord &point){
    if(point >= m_low) m_high = point;
    else{
        m_high = m_low;
        m_low = point;
    }

    return *this;
}

void swap(Line &first, Line &second) noexcept{
    std::swap(first.m_low, second.m_low);
    std::swap(first.m_high, second.m_high);
}

std::ostream &operator<<(std::ostream &os, const Line &line) {
    return os << "L[" << line.m_low << " -- " << line.m_high << "]";
}

size_t std::hash<Line>::operator()(const Line &key) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, key.low());
    boost::hash_combine(seed, key.high());
    return seed;
}

size_t boost::hash<Line>::operator()(const Line &key) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, key.low());
    boost::hash_combine(seed, key.high());
    return seed;
}
