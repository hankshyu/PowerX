
//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        02/21/2025 23:00:39
//  Module Name:        line.hpp
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
//////////////////////////////////////////////////////////////////////////////////｀｀
#ifndef __LINE_H__
#define __LINE_H__

// Dependencies
// 1. C++ STL:
#include <ostream>

// 2. Boost Library:
#include "boost/polygon/polygon.hpp"

// 3. Texo Library:
#include "cord.hpp"


class Line{
private:
    Cord m_low;
    Cord m_high;

public:

    Line();
    explicit Line(const Cord &low, const Cord &high);
    

    bool operator == (const Line &other) const;
    bool operator!=(const Line &other) const;

    bool operator<(const Line &other) const;
    bool operator<=(const Line &other) const;
    bool operator>(const Line &other) const;
    bool operator>=(const Line &other) const;

    Cord get(boost::polygon::direction_1d dir) const;
    Cord low() const;
    Cord high() const;

    void set(boost::polygon::direction_1d dir, const Cord &point);
    Line &low(const Cord &point);
    Line &high(const Cord &point);


    friend void swap(Line &first, Line &second) noexcept;
    friend std::ostream &operator << (std::ostream &os, const Line &line);

};

namespace boost {namespace polygon{
    template <>
    struct geometry_concept<Line> { typedef segment_concept type; };

    template <>
    struct segment_traits<Line> {
        typedef len_t coordinate_type;
        typedef Cord point_type;

        static inline point_type get(const Line& line, direction_1d dir) {
            return line.get(dir);
        }
    };

    template <>
    struct segment_mutable_traits<Line> {
        typedef len_t coordinate_type;
        typedef Cord point_type;

        static inline void set(Line& line, direction_1d dir, const Cord& point) {
            line.set(dir, point);
        }

        static inline Line construct(const Cord& low, const Cord& high) {
            return Line(low, high);
        }
    };

}}

// Line class hash function implementations
namespace std{
    template<>
    struct hash<Line>{
        size_t operator()(const Line &key) const;
    };
} // namespace std

namespace boost{
    template<>
    struct hash<Line>{
        size_t operator()(const Line &key) const;
    };
} // namespace boost

#endif  // #define __LINE_H__