//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        03/02/2025 00:03:44
//  Module Name:        tile.cpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        Basic element for corner stitching, a rectangle with 4 pointers
//                      pointing at neighboring tiles
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//  03/01/2025:         Remove unnecessary exceptions
//
//////////////////////////////////////////////////////////////////////////////////

// Dependencies
// 1. C++ STL:

// 2. Boost Library:
#include "boost/polygon/polygon.hpp"

// 3. Texo Library:
#include "rectangle.hpp"
#include "tile.hpp"


std::ostream &operator << (std::ostream &os, const enum tileType &t){
    switch (t){
        case tileType::BLANK:
            os << "tileType::BLANK";
            break;
        case tileType::BLOCK:
            os << "tileType::BLOCK";
            break;
        case tileType::OVERLAP:
            os << "tileType::OVERLAP";
            break;
        default:
            break;
    }

    return os;
}

Tile::Tile()
    : mType(tileType::BLANK), mRectangle(Rectangle()), rt(nullptr), tr(nullptr), bl(nullptr), lb(nullptr) {
}

Tile::Tile(tileType t, const Rectangle &rect)
    : mType(t), mRectangle(rect), 
    rt(nullptr), tr(nullptr), bl(nullptr), lb(nullptr) {
}

Tile::Tile(tileType t, const Cord &ll, len_t w, len_t h)
    : mType(t), mRectangle(Rectangle(ll.x(), ll.y(), len_t(ll.x() + w), len_t(ll.y() + h))), 
    rt(nullptr), tr(nullptr), bl(nullptr), lb(nullptr) {
}

Tile::Tile(tileType t, const Cord &ll, const Cord &ur)
    : mType(t), mRectangle(Rectangle(ll.x(), ll.y(), ur.x(), ur.y())), 
    rt(nullptr), tr(nullptr), bl(nullptr), lb(nullptr) {
}

bool Tile::operator == (const Tile &comp) const {
    return (mType == comp.getType()) && (mRectangle == comp.getRectangle()) &&
           ((rt == comp.rt) && (tr == comp.tr) && (bl == comp.bl) && (lb == comp.lb));
}

tileType Tile::getType() const {
    return this->mType;
}
Rectangle Tile::getRectangle() const {
    return this->mRectangle;
}

void Tile::setType(tileType type) {
    this->mType = type;
}

void Tile::setRectangle(const Rectangle &rec) {
    this->mRectangle = rec;
}

len_t Tile::getWidth() const {
    return rec::getWidth(this->mRectangle);
}
len_t Tile::getHeight() const {
    return rec::getHeight(this->mRectangle);
};

area_t Tile::getArea() const {
    return rec::getArea(this->mRectangle);
}

double Tile::calculateAspectRatio() const {
    return rec::calculateAspectRatio(this->mRectangle);
}

FCord Tile::calculateCentre() const {
    return rec::calculateCentre(this->mRectangle);
}

len_t Tile::getXLow() const {
    return rec::getXL(this->mRectangle);
};
len_t Tile::getXHigh() const {
    return rec::getXH(this->mRectangle);
};
len_t Tile::getYLow() const {
    return rec::getYL(this->mRectangle);
};
len_t Tile::getYHigh() const {
    return rec::getYH(this->mRectangle);
};

Cord Tile::getLowerLeft() const {
    return rec::getLL(this->mRectangle);
};
Cord Tile::getLowerRight() const {
    return rec::getLR(this->mRectangle);
};
Cord Tile::getUpperLeft() const {
    return rec::getUL(this->mRectangle);
};
Cord Tile::getUpperRight() const {
    return rec::getUR(this->mRectangle);
};

std::ostream &operator<<(std::ostream &os, const Tile &tile) {
    os << "T[";
    os << tile.mType << ", " << tile.mRectangle;
    os << "]";
    return os;
}

size_t std::hash<Tile>::operator()(const Tile &key) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, key.getType());
    boost::hash_combine(seed, key.getRectangle());
    boost::hash_combine(seed, key.rt);
    boost::hash_combine(seed, key.tr);
    boost::hash_combine(seed, key.bl);
    boost::hash_combine(seed, key.lb);
    
    return seed;
}

size_t boost::hash<Tile>::operator()(const Tile &key) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, key.getType());
    boost::hash_combine(seed, key.getRectangle());
    boost::hash_combine(seed, key.rt);
    boost::hash_combine(seed, key.tr);
    boost::hash_combine(seed, key.bl);
    boost::hash_combine(seed, key.lb);
    
    return seed;
}







