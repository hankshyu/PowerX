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


class Rectangle{
protected:
    len_t m_xl, m_yl;
    len_t m_xh, m_yh;
    
public:
    
    Rectangle();
    explicit Rectangle(len_t xl, len_t yl, len_t xh, len_t yh);
    explicit Rectangle(const Interval &horizontal_interval, const Interval &vertical_interval);
    
    // customized constructors
    explicit Rectangle(const Cord &ll, len_t width, len_t height);
    explicit Rectangle(const Cord &ll, const Cord &ur);

    bool operator == (const Rectangle &other) const;
    bool operator!=(const Rectangle &other) const;

    Interval get(boost::polygon::orientation_2d orient) const;
    void set(boost::polygon::orientation_2d orient, const Interval &value);

    friend void swap(Rectangle &first, Rectangle &second) noexcept;
    friend std::ostream &operator << (std::ostream &os, const Rectangle &rec);

    // customized member functions
    inline Interval getHorizontal() const { return Interval(m_xl, m_xh); }
    inline Interval getVertical() const { return Interval(m_yl, m_yh); }

    inline len_t getWidth() const { return len_t(m_xh - m_xl); }
    inline len_t getHeight() const { return len_t(m_yh - m_yl); }
    inline len_t getHalfPerimeter() const { return len_t(m_xh + m_yh - m_xl - m_yl); }
    inline len_t getPerimeter() const { return len_t(2*(m_xh + m_yh - m_xl - m_yl)); }
    inline area_t getArea() const { return area_t((m_xh - m_xl)*(m_yh - m_yl)); }
    inline double getAspectRatio() const { return double(m_xh - m_xl)/double(m_yh - m_yl); }

    inline len_t getXL() const { return m_xl; }
    inline len_t getYL() const { return m_yl; }
    inline len_t getXH() const { return m_xh; }
    inline len_t getYH() const { return m_yh; }

    inline Cord getLL() const { return Cord(m_xl, m_yl); }
    inline Cord getLR() const { return Cord(m_xh, m_yl); }
    inline Cord getUL() const { return Cord(m_xl, m_yh); }
    inline Cord getUR() const { return Cord(m_xh, m_yh); }
    inline FCord getCentre() const { return FCord(flen_t((m_xl + m_xh)/2.0), flen_t((m_yl + m_yh)/2.0)); }

};

namespace boost {namespace polygon{
    template <>
    struct geometry_concept<Rectangle> { typedef rectangle_concept type; };

    template <>
    struct rectangle_traits<Rectangle> {
        typedef Cord coordinate_type;
        typedef Interval interval_type;
        static inline interval_type get(const Rectangle& rectangle, orientation_2d orient) {
            return rectangle.get(orient); }
    };

    template <>
    struct rectangle_mutable_traits<Rectangle> {

        static inline void set(Rectangle& rectangle, orientation_2d orient, const Interval& interval) {
            rectangle.set(orient, interval); }

        static inline Rectangle construct(const Interval& interval_horizontal, const Interval& interval_vertical) {
            return Rectangle(interval_horizontal, interval_vertical); }
    };
}}


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

// namespace rec{

//     // Returns true if two objects overlap, parameter considerTouch is true touching at the sides or corners is considered overlap.
//     inline bool hasIntersect(const Rectangle &rec1, const Rectangle &rec2, bool considerTouch){
//         return boost::polygon::intersects(rec1, rec2, considerTouch);
//     }

//     // Returns true if smallRec is contained (completely within) BigRec
//     inline bool isContained(const Rectangle &BigRec, const Rectangle &smallRec){
//         return boost::polygon::contains(BigRec, smallRec, true);
//     }

//     // Returns true if the point is contained inside the rectangle
//     inline bool isContained(const Rectangle &rec, Cord point){
//         Rectangle actualRectangle =  Rectangle(getXL(rec), getYL(rec), getXH(rec) - 1, getYH(rec) - 1);
//         return boost::polygon::contains(actualRectangle, point, true);
//     }
// }

#endif  // __RECTANGLE_H__