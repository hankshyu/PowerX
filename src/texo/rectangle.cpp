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
//  Description:        
//
//////////////////////////////////////////////////////////////////////////////////
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
#include "interval.hpp"
#include "cord.hpp"

Rectangle::Rectangle():m_xl(LEN_T_MIN), m_yl(LEN_T_MIN), m_xh(LEN_T_MAX), m_yh(LEN_T_MAX){

}

Rectangle::Rectangle(len_t xl, len_t yl, len_t xh, len_t yh){
    if(xl <= xh){
        m_xl = xl;
        m_xh = xh;
    }else{
        m_xl = xh;
        m_xh = xl;
    }

    if(yl <= yh){
        m_yl = yl;
        m_yh = yh;
    }else{
        m_yl = yh;
        m_yh = yl;
    }
}
Rectangle::Rectangle(const Interval &horizontal_interval, const Interval &vertical_interval)
    :m_xl(horizontal_interval.low()), m_yl(vertical_interval.low()), m_xh(horizontal_interval.high()), m_yh(vertical_interval.high()) {

}

Rectangle::Rectangle(const Cord &ll, len_t width, len_t height):
    m_xl(ll.x()), m_yl(ll.y()), m_xh(ll.x() + width), m_yh(ll.y() + height) {

}

Rectangle::Rectangle(const Cord &ll, const Cord &ur){
    len_t xl = ll.x();
    len_t xh = ur.x();
    len_t yl = ll.y();
    len_t yh = ur.y();
    
    if(xl <= xh){
        m_xl = xl;
        m_xh = xh;
    }else{
        m_xl = xh;
        m_xh = xl;
    }

    if(yl <= yh){
        m_yl = yl;
        m_yh = yh;
    }else{
        m_yl = yh;
        m_yh = yl;
    }
}

bool Rectangle::operator == (const Rectangle &other) const{
    return (m_xl == other.m_xl) && (m_yl == other.m_yl) && (m_xh == other.m_xh) && (m_yh == other.m_yh);
}

bool Rectangle::operator!=(const Rectangle &other) const{
    return (m_xl != other.m_xl) || (m_yl != other.m_yl) || (m_xh != other.m_xh) || (m_yh != other.m_yh);
}

Interval Rectangle::get(boost::polygon::orientation_2d orient) const{
    if(orient == boost::polygon::orientation_2d_enum::HORIZONTAL) return Interval(m_xl, m_xh);
    else return Interval(m_yl, m_yh);
}

void Rectangle::set(boost::polygon::orientation_2d orient, const Interval &value){
    if(orient == boost::polygon::orientation_2d_enum::HORIZONTAL){
        m_xl = value.low();
        m_xh = value.high();
    }else{
        m_yl = value.low();
        m_yh = value.high();
    }
}

void swap(Rectangle &first, Rectangle &second) noexcept{
    std::swap(first.m_xl, second.m_xl);
    std::swap(first.m_yl, second.m_yl);
    std::swap(first.m_xh, second.m_xh);
    std::swap(first.m_yh, second.m_yh);
}

std::ostream &operator<<(std::ostream &os, const Rectangle &rec) {
    os << "R[(" << rec.m_xl << ", " << rec.m_yl << ") W=";
    os << (rec.m_xh - rec.m_xl) << " H=" << (rec.m_yh - rec.m_yl);
    os << "(" << rec.m_xh << ", " << rec.m_yh << ")]";

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
