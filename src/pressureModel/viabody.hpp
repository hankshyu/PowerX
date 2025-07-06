//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        06/07/2025 16:56:06
//  Module Name:        viaBody.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        An Object Representing a via on the pressure system
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//  2025/06/19:         Add forward-declareation of SoftBody class to resolve
//                      circular dependencies
/////////////////////////////////////////////////////////////////////////////////

#ifndef __VIA_BODY_H__
#define __VIA_BODY_H__

// Dependencies
// 1. C++ STL:
#include <vector>
#include <ostream>
// 2. Boost Library:

// 3. Texo Library:
#include "signalType.hpp"
#include "fpoint.hpp"

// circular dependencies:
class SoftBody;  // Forward declaration

enum class ViaBodyStatus{
    UNKNOWN,
    EMPTY,          // attracts all types of signals
    TOP_OCCUPIED,   
    DOWN_OCCUPIED,  // attracts specific signal (down signal type) of up layer
    BROKEN,         // up preplace signal contradicts with down preplace signal
    UNSTABLE,       // two end has different signaltype
    STABLE          // no attraction force

};

std::ostream& operator<<(std::ostream& os, ViaBodyStatus st);

class ViaBody{
private:
    
    int m_viaLayerIdx;
    flen_t m_x;
    flen_t m_y;

    bool m_isPreplaced;
    bool m_upIsFixed;
    bool m_downIsFixed;
    
public:

    SoftBody *upSoftBody;
    SoftBody *downSoftBody;

    ViaBodyStatus status;

    // EMPTY, P1 ~ P10, UNKNOWN if inconsistent
    SignalType activSigType;
    
    ViaBody(int viaLayerIdx, flen_t x, flen_t y);
    ViaBody(int viaLayerIdx, const FPoint &location);
    
    inline void setIsPeplaced() {this->m_isPreplaced = true;}
    inline void setUpIsFixed() {this->m_upIsFixed = true;}
    inline void setDownIsFixed() {this->m_downIsFixed = true;}

    inline int getViaLayerIdx() const {return this->m_viaLayerIdx;}
    inline int getUpLayerIdx() const {return this->m_viaLayerIdx;}
    inline int getDownLayerIdx() const {return this->m_viaLayerIdx+1;}
    inline flen_t x() const {return this->m_x;}
    inline flen_t y() const {return this->m_y;}
    inline FPoint getLocation() const {return FPoint(this->m_x, this->m_y);}
    inline bool getIsPreplaced() const {return this->m_isPreplaced;}
    inline bool getUpIsFixed() const {return this->m_upIsFixed;}
    inline bool getDownIsFixed() const {return this->m_downIsFixed;}

    
};

#endif // __VIA_BODY_H__