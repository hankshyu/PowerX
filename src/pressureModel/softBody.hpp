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
    
    double m_rawExpectCurrent;
    double m_expectedCurrent;
    farea_t m_initialArea;

public:
    double temp;

    double pressure;
    double surfaceTension;

    // Softbody attraction/repulsion parameters
    double attractionStrength;
    flen_t attractionRadius; // Ra
    double repulsionStrength;
    flen_t repulsionRadius;  // Rr

    // ViaBody attraction/repulsion parameters
    double attractEmptyViaStrength;
    flen_t attractEmptyViaRadius;
    double attractSameViaStrength;
    flen_t attractsameViaRadius;


    std::vector<FPoint> contour;
    std::vector<FPoint> cacheContour;

    
    // cache: [0] = contour[1] - contour[0]
    std::vector<flen_t> cacheContourDX; 
    std::vector<flen_t> cacheContourDY;
    std::vector<flen_t> cacheContourDistance;

    FPolygon shape;
    
    std::vector<ViaBody *> hardVias;

    SoftBody(int id, SignalType sig, double expectCurrent, farea_t initArea);

    inline void setExpectedCurrent (double expectedCurrent) {this->m_expectedCurrent = expectedCurrent;}

    inline int getID() const {return this->m_id;}
    inline SignalType getSigType() const {return this->m_sigType;}
    
    inline double getRawExpectedCurrent() const {return this->m_rawExpectCurrent;}
    inline double getExpectedCurrent() const {return this->m_expectedCurrent;}
    
    inline farea_t getInitialArea() const {return this->m_initialArea;}
    
    void initialiseParameters(flen_t canvasWidth, flen_t canvasHeight, flen_t avgViaSpacing);
    void updateParameters();
    void remeshContour(flen_t minDelta);

};

#endif // __SOFT_BODY_H__