//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        03/12/2025 21:46:20
//  Module Name:        technology.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A container for technology related information
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//
/////////////////////////////////////////////////////////////////////////////////

#ifndef __TECHNOLOGY_H__
#define __TECHNOLOGY_H__

// Dependencies
// 1. C++ STL:
#include <string>
#include <unordered_map>

// 2. Boost Library:

// 3. Texo Library:


class Technology{
private:
    static const std::unordered_map<std::string, std::string> m_standardUnits;

    int m_DieGlobalWirePitch; // nm
    int m_DieGlobalWireWidth; // nm
    int m_DieGlobalWireThickness; // nm
    int m_DieIntermediateWirePitch; // nm
    int m_DieIntermediateWireWidth; // nm
    int m_DieIntermediateWireThickness; // nm
    int m_DieLocalWirePitch; // nm
    int m_DieLocalWireWidth; // nm
    int m_DieLocalWireThickness; // nm
    double m_DieDecapDensity; // nF/mm^2

    int m_MicrobumpPitch; // um
    int m_MicrobumpRadius; // um
    double m_MicrobumpResistance; // mOhm
    double m_MicrobumpInductance; // pH

    int m_InterposerWidth; // um
    int m_InterposerHeight; // um
    int m_InterposerMetalWidth; // um
    int m_InterposerMetalPitch; // um
    int m_InterposerMetalThickness; // um
    int m_InterposerDielectricThickness; // um
    int m_InterposerSubstrateThickness; // um

    int m_TsvPitch; // um
    int m_TsvDepth; // um
    double m_TsvResistance; // mOhm
    double m_TsvInductance; // pH

    int m_C4Radius; // um
    double m_C4Resistance; // mOhm
    double m_C4Inductance; // pH

    double m_PCBInductance; // um
    double m_PCBResistance; // uhm
    double m_PCBDecapInductance; // nH
    double m_PCBDecapCapacitance; // uF
    double m_PCBDecapResistance; // uOhm

    double m_DieMetalResistivity; // nOhm.m
    double m_InterposerMetalResistivity; // nOhm.m
    double m_PermitivityOfFreeSpace; // fF/m
    double m_PermitivityOfDielectric;
    double m_PermeabilityOfVaccum; // uH/m

    public:
    Technology();
    explicit Technology(const std::string &filePath);

    inline int getDieGlobalWirePitch() const {return m_DieGlobalWirePitch;}
    inline int getDieGlobalWireWidth() const {return m_DieGlobalWireWidth;}
    inline int getDieGlobalWireThickness() const {return m_DieGlobalWireThickness;}
    inline int getDieIntermediateWirePitch() const {return m_DieIntermediateWirePitch;}
    inline int getDieIntermediateWireWidth() const {return m_DieIntermediateWireWidth;}
    inline int getDieIntermediateWireThickness() const {return m_DieIntermediateWireThickness;}
    inline int getDieLocalWirePitch() const {return m_DieLocalWirePitch;}
    inline int getDieLocalWireWidth() const {return m_DieLocalWireWidth;}
    inline int getDieLocalWireThickness() const {return m_DieLocalWireThickness;}
    inline double getDieDecapDensity() const {return m_DieDecapDensity;}

    inline int getMicrobumpPitch() const {return m_MicrobumpPitch;}
    inline int getMicrobumpRadius() const {return m_MicrobumpRadius;}
    inline double getMicrobumpResistance() const {return m_MicrobumpResistance;}
    inline double getMicrobumpInductance() const {return m_MicrobumpInductance;}

    inline int getInterposerWidth() const {return m_InterposerWidth;}
    inline int getInterposerHeight() const {return m_InterposerHeight;}
    inline int getInterposerMetalWidth() const {return m_InterposerMetalWidth;}
    inline int getInterposerMetalPitch() const {return m_InterposerMetalPitch;}
    inline int getInterposerMetalThickness() const {return m_InterposerMetalThickness;}
    inline int getInterposerDielectricThickness() const {return m_InterposerDielectricThickness;}
    inline int getInterposerSubstrateThickness() const {return m_InterposerSubstrateThickness;}

    inline int getTsvPitch() const {return m_TsvPitch;}
    inline int getTsvDepth() const {return m_TsvDepth;}
    inline double getTsvResistance() const {return m_TsvResistance;}
    inline double getTsvInductance() const {return m_TsvInductance;}

    inline int getC4Radius() const {return m_C4Radius;}
    inline double getC4Resistance() const {return m_C4Resistance;}
    inline double getC4Inductance() const {return m_C4Inductance;}

    inline double getPCBInductance() const {return m_PCBInductance;}
    inline double getPCBResistance() const {return m_PCBResistance;}
    inline double getPCBDecapInductance() const {return m_PCBDecapInductance;}
    inline double getPCBDecapCapacitance() const {return m_PCBDecapCapacitance;}
    inline double getPCBDecapResistance() const {return m_PCBDecapResistance;}

    inline double getDieMetalResistivity() const {return m_DieMetalResistivity;}
    inline double getInterposerMetalResistivity() const {return m_InterposerMetalResistivity;}
    inline double getPermitivityOfFreeSpace() const {return m_PermitivityOfFreeSpace;}
    inline double getPermitivityOfDielectric() const {return m_PermitivityOfDielectric;}
    inline double getPermeabilityOfVaccum() const {return m_PermeabilityOfVaccum;}
};

#endif // __TECHNOLOGY_H__