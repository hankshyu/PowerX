//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        06/07/2025 17:07:43
//  Module Name:        viaBody.cpp
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

// Dependencies
// 1. C++ STL:

// 2. Boost Library:

// 3. Texo Library:
#include "viaBody.hpp"

std::ostream& operator<<(std::ostream& os, ViaBodyStatus st){
    switch (st) {
    case ViaBodyStatus::UNKNOWN:
        return os << "ViaBodyStatus::UNKNOWN";
        break;
    case ViaBodyStatus::EMPTY:
        return os << "ViaBodyStatus::EMPTY";
        break;
    case ViaBodyStatus::TOP_OCCUPIED:
        return os << "ViaBodyStatus::TOP_OCCUPIED";
        break;
    case ViaBodyStatus::DOWN_OCCUPIED:
        return os << "ViaBodyStatus::DOWN_OCCUPIED";
        break;
    case ViaBodyStatus::BROKEN:
        return os << "ViaBodyStatus::BROKEN";
        break;
    case ViaBodyStatus::UNSTABLE:
        return os << "ViaBodyStatus::UNSTABLE";
        break;
    case ViaBodyStatus::STABLE:
        return os << "ViaBodyStatus::STABLE";
        break;

    default:
        return os;
        break;
    }
}

ViaBody::ViaBody(int viaLayerIdx, flen_t x, flen_t y, SignalType preplacedType)
    : m_viaLayerIdx(viaLayerIdx), m_x(x), m_y(y),
        upIsFixed(false), upSoftBody(nullptr),
        downIsFixed(false), downSoftBody(nullptr), status(ViaBodyStatus::UNKNOWN) {}

ViaBody::ViaBody(int viaLayerIdx, flen_t x, flen_t y)
    : ViaBody(viaLayerIdx, x, y, SignalType::EMPTY) {}

ViaBody::ViaBody(int viaLayerIdx, const FPoint &location, SignalType preplacedType)
    : ViaBody(viaLayerIdx, location.x(), location.y(), preplacedType) {}

ViaBody::ViaBody(int viaLayerIdx, const FPoint &location)
    : ViaBody(viaLayerIdx, location.x(), location.y(), SignalType::EMPTY) {}

