//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        02/21/2025 23:19:18 PM
//  Module Name:        cord.hpp
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
//  02/21/2025:         Change to suggested way to register as as a model of point concept
//                      in boost library. Full implementation of documented members.
//
//  02/22/2025:         Add conversion operator from Cord to FCord
//
//////////////////////////////////////////////////////////////////////////////////

#ifndef __CORD_H__
#define __CORD_H__

// Dependencies
// 1. C++ STL:
#include <ostream>

// 2. Boost Library:
#include "boost/polygon/polygon.hpp"

// 3. Texo Library:
#include "units.hpp"
#include "fcord.hpp"

class Cord {
private:
    len_t m_x;
    len_t m_y;

public:
    Cord();
    explicit Cord(len_t x, len_t y);

    //conversin operator that allows Cord to be casted to FCord
    operator FCord() const;

    bool operator==(const Cord &other) const;
    bool operator!=(const Cord &other) const;

    bool operator<(const Cord &other) const;
    bool operator<=(const Cord &other) const;
    bool operator>(const Cord &other) const;
    bool operator>=(const Cord &other) const;

    len_t get(boost::polygon::orientation_2d orient) const;
    len_t x() const;
    len_t y() const;

    void set(boost::polygon::orientation_2d orient, len_t value);
    void x(len_t value);
    void y(len_t value);

    friend void swap(Cord &first, Cord &second) noexcept;
    friend std::ostream &operator<<(std::ostream &os, const Cord &c);
};


// Boost Polygon specializations for Cord
namespace boost { namespace polygon {
    template <>
    struct geometry_concept<Cord> { typedef point_concept type; };

    template <>
    struct point_traits<Cord> {
        typedef len_t coordinate_type;
        
        static inline coordinate_type get(const Cord& point, orientation_2d orient){
            return point.get(orient);
        }
    };

    template <>
    struct point_mutable_traits<Cord> {
        typedef len_t coordinate_type;

        static inline void set(Cord& point, orientation_2d orient, len_t value){
            point.set(orient, value);
        }

        static inline Cord construct(len_t x_value, len_t y_value){
            return Cord(x_value, y_value);
        }
    };
}}

// Cord class std::hash function implementation 
namespace std {
    template <>
    struct hash<Cord> {
        size_t operator()(const Cord &key) const;
    };

}  // namespace std

len_t calL1Distance(const Cord &c1, const Cord &c2) noexcept;
flen_t calL2Distance(const Cord &c1, const Cord &c2) noexcept;
len_t calDistanceSquared(const Cord &c1, const Cord &c2) noexcept;

#endif  // __CORD_H__