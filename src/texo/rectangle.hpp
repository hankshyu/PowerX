//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        02/24/2025 20:00:13
//  Module Name:        rectangle.hpp
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
//  02/21/2025:         Move most inline functions in rec namespace to Rectangle
//                      member functions.
//
//  02/25/2025:         Change private members to protected for inherited class access
//
//  02/28/2025:         Switch back to typedef implementation
//  02/28/2025:         Change namespace rec functions to const reference
//
//////////////////////////////////////////////////////////////////////////////////｀

#ifndef __RECTANGLE_H__
#define __RECTANGLE_H__

// Dependencies
// 1. C++ STL:
#include <ostream>

// 2. Boost Library:
#include "boost/polygon/polygon.hpp"

// 3. Texo Library:
#include "cord.hpp"
#include "fcord.hpp"
#include "interval.hpp"

typedef boost::polygon::rectangle_data<len_t> Rectangle;

std::ostream &operator << (std::ostream &os, const Rectangle &rec);

// Rectangle class hash function implementations
namespace std {
    template <>
    struct hash<Rectangle> {
        size_t operator()(const Rectangle &key) const;
    };

}  // namespace std

namespace boost {
    template <>
    struct hash<Rectangle> {
        size_t operator()(const Rectangle &key) const;
    };

}  // namespace boost

namespace rec{

    inline len_t getWidth(const Rectangle &rec){
        // return boost::polygon::delta(boost::polygon::horizontal(rec));
        return boost::polygon::delta(rec, boost::polygon::orientation_2d_enum::HORIZONTAL);
    }

    inline len_t getHeight(const Rectangle &rec){
        // return boost::polygon::delta(boost::polygon::vertical(rec));
        return boost::polygon::delta(rec, boost::polygon::orientation_2d_enum::VERTICAL);

    }

    inline len_t getHalfPerimeter(const Rectangle &rec){
        return boost::polygon::half_perimeter(rec);
    }

    inline len_t getPerimeter(const Rectangle &rec){
        return boost::polygon::perimeter(rec);
    }

    inline area_t getArea(const Rectangle &rec){
        return boost::polygon::area(rec);
    }

    // modified by ryan : change to const reference
    inline double calculateAspectRatio(const Rectangle& rec){
        return double(boost::polygon::delta(boost::polygon::horizontal(rec))) /
                double(boost::polygon::delta(boost::polygon::vertical(rec)));
    }
    inline len_t getXL(const Rectangle &rec){
        return boost::polygon::xl(rec);
    } 

    inline len_t getXH(const Rectangle &rec){
        return boost::polygon::xh(rec);
    } 

    inline len_t getYL(const Rectangle &rec){
        return boost::polygon::yl(rec);
    } 

    inline len_t getYH(const Rectangle &rec){
        return boost::polygon::yh(rec);
    } 

    inline Cord getLL(const Rectangle &rec){
        return boost::polygon::ll(rec);
    }

    inline Cord getLR(const Rectangle &rec){
        return boost::polygon::lr(rec);
    }

    inline Cord getUL(const Rectangle &rec){
        return boost::polygon::ul(rec);
    }

    inline Cord getUR(const Rectangle &rec){
        return boost::polygon::ur(rec);
    }
    // Returns true if two objects overlap, parameter considerTouch is true touching at the sides or corners is considered overlap.
    inline bool hasIntersect(const Rectangle &rec1, const Rectangle &rec2, bool considerTouch = false){
        return boost::polygon::intersects(rec1, rec2, considerTouch);
    }

    // Returns true if smallRec is contained (completely within) BigRec
    inline bool isContained(const Rectangle &BigRec, const Rectangle &smallRec, bool considerTouch = true){
        return boost::polygon::contains(BigRec, smallRec, considerTouch);
    }

    // Returns true if the point is contained inside the rectangle
    // inline bool isContained(const Rectangle &rec, const Cord &point){
    //     Rectangle actualRectangle =  Rectangle(getXL(rec), getYL(rec), getXH(rec) - 1, getYH(rec) - 1);
    //     return boost::polygon::contains(actualRectangle, point, true);
    // }

    // Calculates the centre coordinate of the Rectangle, centre coordinate my be float number, stored with double data type
    inline FCord calculateCentre(const Rectangle &rec){
        flen_t centreX = flen_t(boost::polygon::xl(rec) + boost::polygon::xh(rec))/2;
        flen_t centreY = flen_t(boost::polygon::yl(rec) + boost::polygon::yh(rec))/2;
        return FCord(centreX, centreY);
    }
}

#endif  // __RECTANGLE_H__