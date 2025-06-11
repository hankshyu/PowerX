//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        02/24/2025 20:17:37
//  Module Name:        tile.cpp
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
#include <ostream>

// 2. Boost Library:
#include "boost/polygon/polygon.hpp"

// 3. Texo Library:
#include "tile.hpp"

std::ostream &operator << (std::ostream &os, const eTileType &t){
    switch (t){
        case eTileType::EMPTY:
            os << "eTileType::EMPTY";
        case eTileType::BLOCK:
            os << "eTileType::BLOCK";
            break;
        case eTileType::BLANK:
            os << "eTileType::BLANK";
            break;
        case eTileType::OVERLAP:
            os << "eTileType::OVERLAP";
            break;
        default:
            break;
    }

    return os;
}

Tile::Tile(): Rectangle(), 
    m_type(eTileType::EMPTY), rt(nullptr), tr(nullptr), bl(nullptr), lb(nullptr){

}

Tile::Tile(const eTileType &t, const Cord &ll, len_t w, len_t h): Rectangle(ll, w, h), 
    m_type(t), rt(nullptr), tr(nullptr), bl(nullptr), lb(nullptr){

}

Tile::Tile(const eTileType &t, const Cord &ll, const Cord &ur): Rectangle(ll, ur),
    m_type(t), rt(nullptr), tr(nullptr), bl(nullptr), lb(nullptr){

}

Tile::Tile(len_t xl, len_t yl, len_t xh, len_t yh): Rectangle(xl, yl, xh, yh),
    m_type(eTileType::EMPTY), rt(nullptr), tr(nullptr), bl(nullptr), lb(nullptr){

}

Tile::Tile(const Interval &horizontal_interval, const Interval &vertical_interval): Rectangle(horizontal_interval, vertical_interval),
    m_type(eTileType::EMPTY), rt(nullptr), tr(nullptr), bl(nullptr), lb(nullptr){

}

bool Tile::operator == (const Tile &other) const {
    return (m_xl == other.m_xl) && (m_yl == other.m_yl) && (m_xh == other.m_xh) && (m_yh == other.m_yh)
        && (m_type == other.m_type) && (rt == other.rt) && (tr == other.tr) && (bl == other.bl) && (lb == other.lb);
}

bool Tile::operator != (const Tile &other) const {
    return (m_xl != other.m_xl) || (m_yl != other.m_yl) || (m_xh != other.m_xh) || (m_yh != other.m_yh)
        || (m_type != other.m_type) || (rt != other.rt) || (tr != other.tr) || (bl != other.bl) || (lb != other.lb);
}

eTileType Tile::getType() const {
    return m_type;
}
void Tile::setType(const eTileType &newType){
    m_type = newType;
}

void swap(Tile &first, Tile &second) noexcept{
    std::swap(first.m_xl, second.m_xl);
    std::swap(first.m_yl, second.m_yl);
    std::swap(first.m_xh, second.m_xh);
    std::swap(first.m_yh, second.m_yh);
    std::swap(first.m_type, second.m_type);
    std::swap(first.rt, second.rt);
    std::swap(first.tr, second.tr);
    std::swap(first.bl, second.bl);
    std::swap(first.lb, second.lb);
}

std::ostream &operator<<(std::ostream &os, const Tile &t) {

    os << "T[" << t.m_type;
    os << ", (" << t.m_xl << ", " << t.m_yl << ") W=";
    os << (t.m_xh - t.m_xl) << " H=" << (t.m_yh - t.m_yl);
    os << "(" << t.m_xh << ", " << t.m_yh << ")]";

    return os;
}

size_t std::hash<Tile>::operator()(const Tile &key) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, key.get(boost::polygon::orientation_2d_enum::HORIZONTAL));
    boost::hash_combine(seed, key.get(boost::polygon::orientation_2d_enum::VERTICAL));
    boost::hash_combine(seed, key.getType());
    boost::hash_combine(seed, key.rt);
    boost::hash_combine(seed, key.tr);
    boost::hash_combine(seed, key.bl);
    boost::hash_combine(seed, key.lb);
    return seed;
}

size_t boost::hash<Tile>::operator()(const Tile &key) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, key.get(boost::polygon::orientation_2d_enum::HORIZONTAL));
    boost::hash_combine(seed, key.get(boost::polygon::orientation_2d_enum::VERTICAL));
    boost::hash_combine(seed, key.getType());
    boost::hash_combine(seed, key.rt);
    boost::hash_combine(seed, key.tr);
    boost::hash_combine(seed, key.bl);
    boost::hash_combine(seed, key.lb);
    return seed;
}
