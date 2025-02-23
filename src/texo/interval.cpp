//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        02/23/2025 12:31:54
//  Module Name:        interval.cpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A len_t data type of boost::interval_concept implementation
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//  02/21/2025:         First implementation Interval member functions, using rule of zero
//
//////////////////////////////////////////////////////////////////////////////////

// Dependencies
// 1. C++ STL:

// 2. Boost Library:
#include "boost/functional/hash.hpp"

// 3. Texo Library:
#include "units.hpp"
#include "interval.hpp"


Interval::Interval():m_low(LEN_T_MIN), m_high(LEN_T_MAX){

}

Interval::Interval(len_t low, len_t high){
    if(low <= high){
        m_low = low;
        m_high = high;
    }else{
        m_low = high;
        m_high = low;
    }
}

bool Interval::operator==(const Interval &other) const{
    return (m_low == other.m_low) && (m_high == other.m_high);
}

bool Interval::operator!=(const Interval &other) const{
    return (m_low != other.m_low) || (m_high != other.m_high);
}

bool Interval::operator<(const Interval &other) const{
    return (m_low != other.m_low)? (m_low < other.m_low) : (m_high < other.m_high);
}

bool Interval::operator<=(const Interval &other) const{
    return (m_low != other.m_low)? (m_low < other.m_low) : (m_high <= other.m_high);
}

bool Interval::operator>(const Interval &other) const{
    return (m_low != other.m_low)? (m_low > other.m_low) : (m_high > other.m_high);
}

bool Interval::operator>=(const Interval &other) const{
    return (m_low != other.m_low)? (m_low > other.m_low) : (m_high >= other.m_high);
}


len_t Interval::get(boost::polygon::direction_1d dir) const{
    return (dir == boost::polygon::direction_1d_enum::LOW)? m_low : m_high;
}

len_t Interval::low() const{
    return m_low;
}

len_t Interval::high() const{
    return m_high;
}

void Interval::set(boost::polygon::direction_1d dir, len_t value){
    if(dir == boost::polygon::direction_1d_enum::LOW){
        if(value <= m_high) m_low = value;
        else m_low = m_high = value;
    }else{ // dir == boost::polygon::direction_1d_enum::HIGH
        if(value >= m_low) m_high = value;
        else m_low = m_high = value;
    }
}

Interval &Interval::low(len_t value){
    if(value <= m_high) m_low = value;
    else m_low = m_high = value;

    return *this;
}

Interval &Interval::high(len_t value){
    if(value >= m_low) m_high = value;
    else m_low = m_high = value;

    return *this;
}

void swap(Interval &first, Interval &second) noexcept{
    std::swap(first.m_low, second.m_low);
    std::swap(first.m_high, second.m_high);
}

std::ostream &operator<<(std::ostream &os, const Interval &intv){
    return os << "I[L: " << intv.m_low << " H: " << intv.m_high << "]";
}

size_t std::hash<Interval>::operator()(const Interval &key) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, key.low());
    boost::hash_combine(seed, key.high());
    return seed;
}
