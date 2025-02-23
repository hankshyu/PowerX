//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        02/22/2025 22:47:43
//  Module Name:        fcord.hpp
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
//  02/22/2025:         Change to suggested way to register as as a model of point concept
//                      in boost library. Full implementation of documented members.
//
//////////////////////////////////////////////////////////////////////////////////

#ifndef __FCORD_H__
#define __FCORD_H__

// Dependencies
// 1. C++ STL:
#include <ostream>

// 2. Boost Library:
#include "boost/polygon/polygon.hpp"

// 3. Texo Library:
#include "units.hpp"


class FCord {
private:
    flen_t m_x;
    flen_t m_y;

public:
    FCord();
    explicit FCord(flen_t x, flen_t y);
    
    bool operator==(const FCord &other) const;
    bool operator!=(const FCord &other) const;

    bool operator<(const FCord &other) const;
    bool operator<=(const FCord &other) const;
    bool operator>(const FCord &other) const;
    bool operator>=(const FCord &other) const;

    flen_t get(boost::polygon::orientation_2d orient) const;
    flen_t x() const;
    flen_t y() const;

    void set(boost::polygon::orientation_2d orient, flen_t value);
    void x(flen_t value);
    void y(flen_t value);

    friend void swap(FCord &first, FCord &second) noexcept;
    friend std::ostream &operator<<(std::ostream &os, const FCord &c);
};
    
    
// Boost Polygon specializations for FCord
namespace boost { namespace polygon {
    template <>
    struct geometry_concept<FCord> { typedef point_concept type; };

    template <>
    struct point_traits<FCord> {
        typedef flen_t coordinate_type;
        static inline coordinate_type get(const FCord& point, orientation_2d orient){
            return point.get(orient);
        }
    };

    template <>
    struct point_mutable_traits<FCord> {
        typedef flen_t coordinate_type;

        static inline void set(FCord& point, orientation_2d orient, flen_t value){
            point.set(orient, value);
        }

        static inline FCord construct(int x_value, int y_value){
            return FCord(x_value, y_value);
        }
    };

}}
    
// Cord class std::hash function implementation 
namespace std {
    template <>
    struct hash<FCord> {
        size_t operator()(const FCord &key) const;
    };

}  // namespace std

flen_t calL1Distance(const FCord &c1, const FCord &c2) noexcept;
flen_t calL2Distance(const FCord &c1, const FCord &c2) noexcept;
flen_t calDistanceSquared(const FCord &c1, const FCord &c2) noexcept;

#endif  // __FCORD_H__