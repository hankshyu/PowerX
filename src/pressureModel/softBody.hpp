//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        06/04/2025 17:09:56
//  Module Name:        softBody.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        The object that expands, shrink and merge due to
//                      pressure-driven forces. 
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//  2025/06/19:         Change hardVias vector to viaBody pointer type, has circular
//                      dependencies with SoftBody type, use forward-declaration
/////////////////////////////////////////////////////////////////////////////////

#ifndef __SOFT_BODY_H__
#define __SOFT_BODY_H__

// Dependencies
// 1. C++ STL:
#include <vector>
// 2. Boost Library:

// 3. Texo Library:
#include "units.hpp"
#include "signalType.hpp"

#include "fpoint.hpp"
#include "fbox.hpp"
#include "fpolygon.hpp"
#include "fmultipolygon.hpp"
#include "viaBody.hpp"

// circular dependencies:
class ViaBody;  // Forward declaration

class SoftBody{
private:
    int m_id;
    SignalType m_sigType;
    
    double m_expectCurrent;
    farea_t m_initialArea;

public:
    double pressure;
    std::vector<FPoint> contour;
    FPolygon shape;
    
    std::vector<ViaBody *> hardVias;

    SoftBody(int id, SignalType sig, double expectCurrent, farea_t initArea);

    inline int getID() const {return this->m_id;}
    inline SignalType getSigType() const {return this->m_sigType;}
    inline double getExpectCurrent() const {return this->m_expectCurrent;}
    inline farea_t getInitialArea() const {return this->m_initialArea;}
    
    double calculatePressure() const;
    void remeshContour(flen_t minDelta);

};

#endif // __SOFT_BODY_H__