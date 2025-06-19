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
#include "softBody.hpp"

enum class ViaBodyStatus{
    UNKNOWN,
    EMPTY,          // attracts all types of signals
    TOP_OCCUPIED,   // attracts specific signal (top signal type) of down layer
    DOWN_OCCUPIED,  // attracts specific signal (down signal type) of up layer
    BROKEN,         // up preplace signal contradicts with down preplace signal
    UNSTABLE,       // two end has different signaltype
    STABLE          // no attraction force

};

std::ostream& operator<<(std::ostream& os, ViaBodyStatus st);

class ViaBody{
private:
    

    SignalType preplacedType;
    
int m_viaLayerIdx;
    flen_t m_x;
    flen_t m_y;
    
public:
    bool upIsFixed;
    SoftBody *upSoftBody;

    bool downIsFixed;
    SoftBody *downSoftBody;

    ViaBodyStatus status;
    
    ViaBody(int viaLayerIdx, flen_t x, flen_t y, SignalType preplacedType);
    ViaBody(int viaLayerIdx, flen_t x, flen_t y);
    ViaBody(int viaLayerIdx, const FPoint &location, SignalType preplacedType);
    ViaBody(int viaLayerIdx, const FPoint &location);
    
    inline int getViaLayerIdx() const {return this->m_viaLayerIdx;}
    inline int getUpLayerIdx() const {return this->m_viaLayerIdx;}
    inline int getDownLayerIdx() const {return this->m_viaLayerIdx+1;}
    
    inline flen_t x() const {return m_x;}
    inline flen_t y() const {return m_y;}
    inline FPoint getLocation() const {return FPoint(m_x, m_y);}

};

#endif // __VIA_BODY_H__