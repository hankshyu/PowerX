//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        03/18/2025 23:55:55
//  Module Name:        dqCktExtractor.cpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        Calculates equivalent circuit components according to
//                      technology files
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//
/////////////////////////////////////////////////////////////////////////////////


// Dependencies
// 1. C++ STL:
#include <cmath>
#include <iostream>


// 2. Boost Library:

// 3. Texo Library:
#include "technology.hpp"
#include "eqCktExtractor.hpp"


EqCktExtractor::EqCktExtractor(const Technology &tch): m_Technology(tch){

    double InterposerS = tch.getInterposerMetalPitch();
    double InterposerW = tch.getInterposerMetalWidth();
    double InterposerH = tch.getInterposerMetalThickness();
    double InterposerSP = InterposerS - 2*InterposerW;

    m_InterposerResistance = (tch.getInterposerMetalResistivity() / InterposerH) * (InterposerS / (4*InterposerW));
    m_InterposerInductance = InterposerS * (0.13 * std::exp(-InterposerS/45) + 0.14 * std::log(InterposerS/InterposerW) + 0.07);
    
    m_InterposerCapacitanceI = (tch.getPermitivityOfDielectric() / 1000) * (((44 - 28*InterposerH) * InterposerW * InterposerW) + 
        ((280*InterposerH + 0.8*InterposerS - 64) * InterposerW) + 12*InterposerS - 1500*InterposerH + 1700);
    
    double tmp = std::log(InterposerS/InterposerSP) + std::exp(-1.0/3.0);
    m_InterposerCapacitanceF = tch.getPermitivityOfFreeSpace()*tch.getPermitivityOfDielectric()*0.001 * 
        (((4*InterposerS * InterposerW * tmp) / (InterposerW*M_PI + 2*InterposerH*tmp))+ (2*InterposerS/M_PI) * sqrt((2*InterposerH)/InterposerSP));
    
    m_InterposerCapacitanceA = (tch.getPermitivityOfDielectric() / 1000) * ((4.427*InterposerW*InterposerW/InterposerH) +
        (96 - 56*InterposerH)*InterposerW + 20*InterposerH - 41);

    m_InterposerCapacitanceCenterCell = m_InterposerCapacitanceI + m_InterposerCapacitanceF;
    m_InterposerCapacitanceEdgeCell = m_InterposerCapacitanceCenterCell + m_InterposerCapacitanceA;
    m_InterposerCapacitanceCornerCell = m_InterposerCapacitanceEdgeCell + m_InterposerCapacitanceA;

    double viaPitch = InterposerS;
    double viaLength = tch.getInterposerDielectricThickness();
    double viaRadius = InterposerW/2;
    double viaLoverR = viaLength / viaRadius;

    m_InterposerViaResistance = tch.getInterposerMetalResistivity() * 1000 * viaLength / (M_PI * viaRadius * viaRadius);
    
    m_InterposerViaInductance = (tch.getPermeabilityOfVaccum() * viaRadius * (1 + viaLength / viaRadius)) / 2;


}