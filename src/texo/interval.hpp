//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        02/23/2025 00:15:48
//  Module Name:        interval.hpp
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
//  02/21/2025:         First implementation of interval concept in Boost Library using 
//                      self constructed class. Full implementation of documented members.
//
//////////////////////////////////////////////////////////////////////////////////

#ifndef __INTERVAL_H__
#define __INTERVAL_H__

// Dependencies
// 1. C++ STL:
#include <ostream>

// 2. Boost Library:
#include "boost/polygon/polygon.hpp"

// 3. Texo Library:
#include "units.hpp"

class Interval {
private:
    len_t m_low;
    len_t m_high;

public:
    Interval();
    explicit Interval(len_t low, len_t high);

    bool operator==(const Interval &other) const;
    bool operator!=(const Interval &other) const;

    bool operator<(const Interval &other) const;
    bool operator<=(const Interval &other) const;
    bool operator>(const Interval &other) const;
    bool operator>=(const Interval &other) const;

    len_t get(boost::polygon::direction_1d dir) const;
    len_t low() const;
    len_t high() const;

    void set(boost::polygon::direction_1d dir, len_t value);
    Interval &low(len_t value);
    Interval &high(len_t value);

    friend void swap(Interval &first, Interval &second) noexcept;
    friend std::ostream &operator<<(std::ostream &os, const Interval &intv);

};

namespace boost {namespace polygon{
    template <>
    struct geometry_concept<Interval> { typedef interval_concept type; };

    template <>
    struct interval_traits<Interval> {
        typedef len_t coordinate_type;
        
        static inline coordinate_type get(const Interval& interval, direction_1d dir) {
            return interval.get(dir);
        }
    };

    template<>
    struct interval_mutable_traits<Interval> {
        typedef len_t coordinate_type;

        static inline void set(Interval& interval, direction_1d dir, len_t value){
            interval.set(dir, value);
        }

        static inline Interval construct(len_t low_value, len_t high_value){
            return Interval(low_value, high_value);
        }
    };

}}

// Interval class std::hash function implementation 
namespace std {
    template <>
    struct hash<Interval> {
        size_t operator()(const Interval &key) const;
    };

}  // namespace std


#endif  // __INTERVAL_H__