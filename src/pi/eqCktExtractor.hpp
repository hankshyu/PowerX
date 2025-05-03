//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        03/18/2025 13:33:33
//  Module Name:        dqCktExtractor.hpp
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

#ifndef __EQCKTEXTRACTOR_H__
#define __EQCKTEXTRACTOR_H__

// Dependencies
// 1. C++ STL:
#include <string>


// 2. Boost Library:

// 3. Texo Library:
#include "technology.hpp"



class EqCktExtractor{
private:
    const Technology &m_Technology;

    double m_InterposerResistance;
    double m_InterposerInductance;
    
    double m_InterposerCapacitanceI;
    double m_InterposerCapacitanceF;
    double m_InterposerCapacitanceA;
    double m_InterposerCapacitanceCenterCell;
    double m_InterposerCapacitanceEdgeCell;
    double m_InterposerCapacitanceCornerCell;

    double m_InterposerConductance;
 
    double m_InterposerViaResistance;
    double m_InterposerViaInductance;

    // inline std::string nodeToString(int z, int x, int y) const {return std::to_string(z)+"_"+std::to_string(x)+"_"+std::to_string(y);}

public:
    EqCktExtractor() = delete;
    explicit EqCktExtractor(const Technology &tch);

    // mOhm
    inline double getInterposerResistance() const {return m_InterposerResistance;}
    // pH
    inline double getInterposerInductance() const {return m_InterposerInductance;}
    // fF
    inline double getInterposerCapacitanceCenterCell() const {return m_InterposerCapacitanceCenterCell;}
    // fF
    inline double getInterposerCapacitanceEdgeCell() const {return m_InterposerCapacitanceEdgeCell;}
    // fF
    inline double getInterposerCapacitanceCornerCell() const {return m_InterposerCapacitanceCornerCell;}
    
    //mOhm
    inline double getInterposerViaResistance() const {return m_InterposerViaResistance;}
    //pH
    inline double getInterposerViaInductance() const {return m_InterposerViaInductance;}

};

#endif // __EQCKTEXTRACTOR_H__