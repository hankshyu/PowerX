//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        03/20/2025 17:45:40
//  Module Name:        timeProfiler.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        Profiles and logs the execution time of program sections 
//                      for analysis
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//
//////////////////////////////////////////////////////////////////////////////////

#ifndef __TIMEPROFILER_H__
#define __TIMEPROFILER_H__

// Dependencies
// 1. C++ STL:
#include <string>
#include <chrono>
#include <vector>
#include <unordered_map>

// 2. Boost Library:


// 3. Texo Library:

typedef std::string timeSpanName;

struct timeSpan{
    int periodCount = 0;
    std::vector<std::chrono::high_resolution_clock::time_point> startingPoints;
    std::vector<std::chrono::high_resolution_clock::time_point> endingPoints;
};

class TimeProfiler{
    std::vector<timeSpanName> m_timeSpans;
    std::unordered_map<timeSpanName, timeSpan> m_timeSpanMap;

public:
    
    void startTimer(const timeSpanName &newTimeSpan);
    void pauseTimer(const timeSpanName &timeSpan);

    void printTimingReport() const;
};

#endif // __TIMEPROFILER_H__