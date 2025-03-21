//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        03/20/2025 18:02:25
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

// Dependencies
// 1. C++ STL:
#include <string>
#include <chrono>
#include <vector>
#include <unordered_map>
#include <algorithm>

// 2. Boost Library:


// 3. Texo Library:
#include "timeProfiler.hpp"


void TimeProfiler::startTimer(const timeSpanName &newTimeSpan){

    if(std::find(m_timeSpans.begin(), m_timeSpans.end(), newTimeSpan) == m_timeSpans.end()){
        // this is a new timeSpan
        m_timeSpans.push_back(newTimeSpan);
        m_timeSpanMap[newTimeSpan].periodCount = 0;
    }

    m_timeSpanMap[newTimeSpan].startingPoints.push_back(std::chrono::high_resolution_clock::now());
}

void TimeProfiler::pauseTimer(const timeSpanName &oldTimeSpan) {
    std::unordered_map<timeSpanName, timeSpan>::iterator it = m_timeSpanMap.find(oldTimeSpan);
    if (it != m_timeSpanMap.end() && (!it->second.startingPoints.empty())) {
        it->second.endingPoints.push_back(std::chrono::high_resolution_clock::now());
        it->second.periodCount++;
    }
}

void TimeProfiler::printTimingReport() const {

    // count total duration and cache each duration in map
    double totalDuration = 0;
    std::unordered_map<timeSpanName, double> timeSpanToDuration;
    for(int i = 0; i < m_timeSpans.size(); ++i){
        const timeSpan &ts = m_timeSpanMap.at(m_timeSpans[i]);

        std::chrono::duration<double, std::milli> duration(0);

        for(int j = 0; j < ts.periodCount; ++j){
            duration += (ts.endingPoints[j] - ts.startingPoints[j]);
        }
        totalDuration += duration.count();
        timeSpanToDuration[m_timeSpans[i]] = duration.count();
    }

    printf("╔══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║ Stage                     │     Runtime (s)      |                                                                           ║\n");
    printf("╟───────────────────────────│──────────────────────│───────────────────────────────────────────────────────────────────────────╢\n");
    for(int i = 0; i < m_timeSpans.size(); ++i){
        const timeSpan &ts = m_timeSpanMap.at(m_timeSpans[i]);
        std::string StageName = " " + m_timeSpans[i];

        double durationS = timeSpanToDuration[m_timeSpans[i]];
        double durationPercentage = durationS / totalDuration;
        durationS = durationS / 1000.0;
        

        printf("║%-27s│ %11.3lf (%5.2lf%%) │%75s║\n", StageName.c_str(), durationS, durationPercentage,  "");

    }
    printf("╟───────────────────────────│──────────────────────│───────────────────────────────────────────────────────────────────────────╢\n");
    printf("║ Summary                   │ %11.3lf          │%75s║\n", totalDuration/1000.0, "");
    printf("╚══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝\n");

}
 