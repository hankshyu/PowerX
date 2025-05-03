//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        03/13/2025 22:11:12
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

// Dependencies
// 1. C++ STL:
#include <iostream>
#include <cassert>
#include <string>
#include <regex>
#include <fstream>
#include <algorithm>
#include <cmath>

// 2. Boost Library:

// 3. Texo Library:
#include "technology.hpp"

const std::unordered_map<std::string, std::string> Technology::m_standardUnits = {
    {"DIE_GLOBAL_WIRE_PITCH", "nm"},
    {"DIE_GLOBAL_WIRE_WIDTH", "nm"},
    {"DIE_GLOBAL_WIRE_THICKNESS", "nm"},

    {"DIE_INTERMEDIATE_WIRE_PITCH", "nm"},
    {"DIE_INTERMEDIATE_WIRE_WIDTH", "nm"},
    {"DIE_INTERMEDIATE_WIRE_THICKNESS", "nm"},

    {"DIE_LOCAL_WIRE_PITCH", "nm"},
    {"DIE_LOCAL_WIRE_WIDTH", "nm"},
    {"DIE_LOCAL_WIRE_THICKNESS", "nm"},

    {"DIE_DECAP_DENSITY", "nF/mm^2"},

    {"MICROBUMP_PITCH", "um"},
    {"MICROBUMP_RADIUS", "um"},
    {"MICROBUMP_RESISTANCE", "mOhm"},
    {"MICROBUMP_INDUCTANCE", "pH"},

    {"INTERPOSER_WIDTH", "um"},
    {"INTERPOSER_HEIGHT", "um"},
    {"INTERPOSER_METAL_WIDTH", "um"},
    {"INTERPOSER_METAL_PITCH", "um"},
    {"INTERPOSER_METAL_THICKNESS", "um"},
    {"INTERPOSER_DIELECTRIC_THICKNESS", "um"},
    {"INTERPOSER_SUBSTRATE_THICKNESS", "um"},
        
    {"TSV_PITCH", "um"},
    {"TSV_DEPTH", "um"},
    {"TSV_RESISTANCE", "mOhm"},
    {"TSV_INDUCTANCE", "pH"},

    {"C4_RADIUS", "um"},
    {"C4_RESISTANCE", "mOhm"},
    {"C4_INDUCTANCE", "pH"},

    {"PCB_INDUCTANCE", "pH"},
    {"PCB_RESISTANCE", "uOhm"},
    {"PCB_DECAP_INDUCTANCE", "nH"},
    {"PCB_DECAP_CAPACITANCE", "uF"},
    {"PCB_DECAP_RESISTANCE", "uOhm"},

    {"DIE_METAL_RESISTIVITY", "nOhm.m"},
    {"INTERPOSER_METAL_RESISTIVITY", "nOhm.m"},
    {"PERMITIVITY_OF_FREE_SPACE", "fF/m"},
    {"PERMITIVITY_OF_DIELECTRIC", ""},
    {"PERMEABILITY_OF_VACCUM", "uH/m"},
    {"LOSS_TANGENT", ""}

};


Technology::Technology():
    m_DieGlobalWirePitch(39500), m_DieGlobalWireWidth(17500), m_DieGlobalWireThickness(7000),
    m_DieIntermediateWirePitch(560), m_DieIntermediateWireWidth(280), m_DieIntermediateWireThickness(506),
    m_DieLocalWirePitch(160), m_DieLocalWireWidth(80), m_DieLocalWireThickness(144), m_DieDecapDensity(355),
    m_MicrobumpPitch(100), m_MicrobumpRadius(20), m_MicrobumpResistance(30.9), m_MicrobumpInductance(11.1),
    m_InterposerWidth(10800), m_InterposerHeight(10800), 
    m_InterposerMetalWidth(40), m_InterposerMetalPitch(100), m_InterposerMetalThickness(1),
    m_InterposerDielectricThickness(1), m_InterposerSubstrateThickness(100),
    m_TsvPitch(40), m_TsvDepth(100), m_TsvResistance(54.2), m_TsvInductance(77.78),
    m_C4Radius(120), m_C4Resistance(14.3), m_C4Inductance(11),
    m_PCBInductance(21), m_PCBResistance(166), 
    m_PCBDecapInductance(19.54), m_PCBDecapCapacitance(240), m_PCBDecapResistance(166),
    m_DieMetalResistivity(1.18), 
    m_InterposerMetalResistivity(2.2),
    m_PermitivityOfFreeSpace(8.854),
    m_PermitivityOfDielectric(3.9),
    m_PermeabilityOfVaccum(1.256637), 
    m_LossTangent(0.002) {

}

Technology::Technology(const std::string &filePath){
    Technology();
    std::unordered_map<char, int> magnitude_map = {{'f', -15}, {'p', -12}, {'n', -9}, {'u', -6}, {'m', -3}, {'c', -2}};
    std::ifstream file(filePath);
    assert(file.is_open());

    std::string lineBuffer;
    while(std::getline(file, lineBuffer)){
        if (lineBuffer.empty()|| lineBuffer.find("#") == 0) continue; // whole line comment
        
        size_t comment_pos = lineBuffer.find_first_of("#");
        if (comment_pos != std::string::npos) {
                lineBuffer = lineBuffer.substr(0, comment_pos);  // Remove everything after "#"
        }

        
        // process the line

        std::string key, value, unit;
        std::regex pattern(R"(\s*([\w_]+)\s*=\s*(-?\d+(?:\.\d+)?(?:[eE][-+]?\d+)?)\s*([\w.\-^/µ]*)\s*)");
        std::smatch match;
        if(!std::regex_match(lineBuffer, match, pattern)){
            std::cout << "[PowerX:TchParser] Unmatch string: " << lineBuffer << std::endl;
            continue;
        }

        key = match[1].str();
        std::transform(key.begin(), key.end(), key.begin(), ::toupper);
        value = match[2].str();
        unit = match[3].str();

        if(m_standardUnits.find(key) == m_standardUnits.end()){
            std::cout << "[PowerX:TchParser] Unmatch parameter: " << key << std::endl;
            continue;
        }

        std::string stdUnit = m_standardUnits.at(key);
        char stdMagnitude;
        char magnitude;
        if(!stdUnit.empty()){
            stdMagnitude = stdUnit[0];
            stdUnit =stdUnit.erase(0, 1);
            
            magnitude = unit[0];
            unit = unit.erase(0, 1);
        }

        if(key == "DIE_GLOBAL_WIRE_PITCH"){
            if((stdUnit != unit) || (magnitude_map.find(magnitude) == magnitude_map.end())){
                std::cout << "[PowerX:TchParser] Unmatch unit for DIE_GLOBAL_WIRE_PITCH: " << magnitude << unit << std::endl;
                continue;
            }
            m_DieGlobalWirePitch = std::stod(value) * std::pow(10, (magnitude_map[magnitude] - magnitude_map[stdMagnitude]));

        }else if(key == "DIE_GLOBAL_WIRE_WIDTH"){
            if((stdUnit != unit) || (magnitude_map.find(magnitude) == magnitude_map.end())){
                std::cout << "[PowerX:TchParser] Unmatch unit for DIE_GLOBAL_WIRE_WIDTH: " << magnitude << unit << std::endl;
                continue;
            }
            m_DieGlobalWireWidth = std::stod(value) * std::pow(10, (magnitude_map[magnitude] - magnitude_map[stdMagnitude]));

        }else if(key == "DIE_GLOBAL_WIRE_THICKNESS"){
            if((stdUnit != unit) || (magnitude_map.find(magnitude) == magnitude_map.end())){
                std::cout << "[PowerX:TchParser] Unmatch unit for DIE_GLOBAL_WIRE_THICKNESS: " << magnitude << unit << std::endl;
                continue;
            }
            m_DieGlobalWireThickness = std::stod(value) * std::pow(10, (magnitude_map[magnitude] - magnitude_map[stdMagnitude]));
        
        }else if(key == "DIE_INTERMEDIATE_WIRE_PITCH"){
            if((stdUnit != unit) || (magnitude_map.find(magnitude) == magnitude_map.end())){
                std::cout << "[PowerX:TchParser] Unmatch unit for DIE_INTERMEDIATE_WIRE_PITCH: " << magnitude << unit << std::endl;
                continue;
            }
            m_DieIntermediateWirePitch = std::stod(value) * std::pow(10, (magnitude_map[magnitude] - magnitude_map[stdMagnitude]));

        }else if(key == "DIE_INTERMEDIATE_WIRE_WIDTH"){
            if((stdUnit != unit) || (magnitude_map.find(magnitude) == magnitude_map.end())){
                std::cout << "[PowerX:TchParser] Unmatch unit for DIE_INTERMEDIATE_WIRE_WIDTH: " << magnitude << unit << std::endl;
                continue;
            }
            m_DieIntermediateWireWidth = std::stod(value) * std::pow(10, (magnitude_map[magnitude] - magnitude_map[stdMagnitude]));

        }else if(key == "DIE_INTERMEDIATE_WIRE_THICKNESS"){
            if((stdUnit != unit) || (magnitude_map.find(magnitude) == magnitude_map.end())){
                std::cout << "[PowerX:TchParser] Unmatch unit for DIE_INTERMEDIATE_WIRE_THICKNESS: " << magnitude << unit << std::endl;
                continue;
            }
            m_DieIntermediateWireThickness = std::stod(value) * std::pow(10, (magnitude_map[magnitude] - magnitude_map[stdMagnitude]));

        }else if(key == "DIE_LOCAL_WIRE_PITCH"){
            if((stdUnit != unit) || (magnitude_map.find(magnitude) == magnitude_map.end())){
                std::cout << "[PowerX:TchParser] Unmatch unit for DIE_LOCAL_WIRE_PITCH: " << magnitude << unit << std::endl;
                continue;
            }
            m_DieLocalWirePitch = std::stod(value) * std::pow(10, (magnitude_map[magnitude] - magnitude_map[stdMagnitude]));

        }else if(key == "DIE_LOCAL_WIRE_WIDTH"){
            if((stdUnit != unit) || (magnitude_map.find(magnitude) == magnitude_map.end())){
                std::cout << "[PowerX:TchParser] Unmatch unit for DIE_LOCAL_WIRE_WIDTH: " << magnitude << unit << std::endl;
                continue;
            }
            m_DieLocalWireWidth = std::stod(value) * std::pow(10, (magnitude_map[magnitude] - magnitude_map[stdMagnitude]));

        }else if(key == "DIE_LOCAL_WIRE_THICKNESS"){
            if((stdUnit != unit) || (magnitude_map.find(magnitude) == magnitude_map.end())){
                std::cout << "[PowerX:TchParser] Unmatch unit for DIE_LOCAL_WIRE_THICKNESS: " << magnitude << unit << std::endl;
                continue;
            }
            m_DieLocalWireThickness = std::stod(value) * std::pow(10, (magnitude_map[magnitude] - magnitude_map[stdMagnitude]));
            
        }else if(key == "DIE_DECAP_DENSITY"){
            if((stdUnit != unit) || (magnitude_map.find(magnitude) == magnitude_map.end())){
                std::cout << "[PowerX:TchParser] Unmatch unit for DIE_DECAP_DENSITY: " << magnitude << unit << std::endl;
                continue;
            }
            m_DieDecapDensity = std::stod(value) * std::pow(10, (magnitude_map[magnitude] - magnitude_map[stdMagnitude]));

        }else if(key == "MICROBUMP_PITCH"){
            if((stdUnit != unit) || (magnitude_map.find(magnitude) == magnitude_map.end())){
                std::cout << "[PowerX:TchParser] Unmatch unit for MICROBUMP_PITCH: " << magnitude << unit << std::endl;
                continue;
            }
            m_MicrobumpPitch = std::stod(value) * std::pow(10, (magnitude_map[magnitude] - magnitude_map[stdMagnitude]));

        }else if(key == "MICROBUMP_RADIUS"){
            if((stdUnit != unit) || (magnitude_map.find(magnitude) == magnitude_map.end())){
                std::cout << "[PowerX:TchParser] Unmatch unit for MICROBUMP_RADIUS: " << magnitude << unit << std::endl;
                continue;
            }
            m_MicrobumpRadius = std::stod(value) * std::pow(10, (magnitude_map[magnitude] - magnitude_map[stdMagnitude]));

        }else if(key == "MICROBUMP_RESISTANCE"){
            if((stdUnit != unit) || (magnitude_map.find(magnitude) == magnitude_map.end())){
                std::cout << "[PowerX:TchParser] Unmatch unit for MICROBUMP_RESISTANCE: " << magnitude << unit << std::endl;
                continue;
            }
            m_MicrobumpResistance = std::stod(value) * std::pow(10, (magnitude_map[magnitude] - magnitude_map[stdMagnitude]));

        }else if(key == "MICROBUMP_INDUCTANCE"){
            if((stdUnit != unit) || (magnitude_map.find(magnitude) == magnitude_map.end())){
                std::cout << "[PowerX:TchParser] Unmatch unit for MICROBUMP_INDUCTANCE: " << magnitude << unit << std::endl;
                continue;
            }
            m_MicrobumpInductance = std::stod(value) * std::pow(10, (magnitude_map[magnitude] - magnitude_map[stdMagnitude]));

        }else if(key == "INTERPOSER_WIDTH"){
            if((stdUnit != unit) || (magnitude_map.find(magnitude) == magnitude_map.end())){
                std::cout << "[PowerX:TchParser] Unmatch unit for INTERPOSER_WIDTH: " << magnitude << unit << std::endl;
                continue;
            }
            m_InterposerWidth = std::stod(value) * std::pow(10, (magnitude_map[magnitude] - magnitude_map[stdMagnitude]));

        }else if(key == "INTERPOSER_HEIGHT"){
            if((stdUnit != unit) || (magnitude_map.find(magnitude) == magnitude_map.end())){
                std::cout << "[PowerX:TchParser] Unmatch unit for INTERPOSER_HEIGHT: " << magnitude << unit << std::endl;
                continue;
            }
            m_InterposerHeight = std::stod(value) * std::pow(10, (magnitude_map[magnitude] - magnitude_map[stdMagnitude]));

        }else if(key == "INTERPOSER_METAL_WIDTH"){
            if((stdUnit != unit) || (magnitude_map.find(magnitude) == magnitude_map.end())){
                std::cout << "[PowerX:TchParser] Unmatch unit for INTERPOSER_METAL_WIDTH: " << magnitude << unit << std::endl;
                continue;
            }
            m_InterposerMetalWidth = std::stod(value) * std::pow(10, (magnitude_map[magnitude] - magnitude_map[stdMagnitude]));

        }else if(key == "INTERPOSER_METAL_PITCH"){
            if((stdUnit != unit) || (magnitude_map.find(magnitude) == magnitude_map.end())){
                std::cout << "[PowerX:TchParser] Unmatch unit for INTERPOSER_METAL_PITCH: " << magnitude << unit << std::endl;
                continue;
            }
            m_InterposerMetalPitch = std::stod(value) * std::pow(10, (magnitude_map[magnitude] - magnitude_map[stdMagnitude]));

        }else if(key == "INTERPOSER_METAL_THICKNESS"){
            if((stdUnit != unit) || (magnitude_map.find(magnitude) == magnitude_map.end())){
                std::cout << "[PowerX:TchParser] Unmatch unit for INTERPOSER_METAL_THICKNESS: " << magnitude << unit << std::endl;
                continue;
            }
            m_InterposerMetalThickness = std::stod(value) * std::pow(10, (magnitude_map[magnitude] - magnitude_map[stdMagnitude]));

        }else if(key == "INTERPOSER_DIELECTRIC_THICKNESS"){
            if((stdUnit != unit) || (magnitude_map.find(magnitude) == magnitude_map.end())){
                std::cout << "[PowerX:TchParser] Unmatch unit for INTERPOSER_DIELECTRIC_THICKNESS: " << magnitude << unit << std::endl;
                continue;
            }
            m_InterposerDielectricThickness = std::stod(value) * std::pow(10, (magnitude_map[magnitude] - magnitude_map[stdMagnitude]));

        }else if(key == "INTERPOSER_SUBSTRATE_THICKNESS"){
            if((stdUnit != unit) || (magnitude_map.find(magnitude) == magnitude_map.end())){
                std::cout << "[PowerX:TchParser] Unmatch unit for INTERPOSER_SUBSTRATE_THICKNESS: " << magnitude << unit << std::endl;
                continue;
            }
            m_InterposerSubstrateThickness = std::stod(value) * std::pow(10, (magnitude_map[magnitude] - magnitude_map[stdMagnitude]));

        }else if(key == "TSV_PITCH"){
            if((stdUnit != unit) || (magnitude_map.find(magnitude) == magnitude_map.end())){
                std::cout << "[PowerX:TchParser] Unmatch unit for TSV_PITCH: " << magnitude << unit << std::endl;
                continue;
            }
            m_TsvPitch = std::stod(value) * std::pow(10, (magnitude_map[magnitude] - magnitude_map[stdMagnitude]));

        }else if(key == "TSV_DEPTH"){
            if((stdUnit != unit) || (magnitude_map.find(magnitude) == magnitude_map.end())){
                std::cout << "[PowerX:TchParser] Unmatch unit for TSV_DEPTH: " << magnitude << unit << std::endl;
                continue;
            }
            m_TsvDepth = std::stod(value) * std::pow(10, (magnitude_map[magnitude] - magnitude_map[stdMagnitude]));

        }else if(key == "TSV_RESISTANCE"){
            if((stdUnit != unit) || (magnitude_map.find(magnitude) == magnitude_map.end())){
                std::cout << "[PowerX:TchParser] Unmatch unit for TSV_RESISTANCE: " << magnitude << unit << std::endl;
                continue;
            }
            m_TsvResistance = std::stod(value) * std::pow(10, (magnitude_map[magnitude] - magnitude_map[stdMagnitude]));

        }else if(key == "TSV_INDUCTANCE"){
            if((stdUnit != unit) || (magnitude_map.find(magnitude) == magnitude_map.end())){
                std::cout << "[PowerX:TchParser] Unmatch unit for TSV_INDUCTANCE: " << magnitude << unit << std::endl;
                continue;
            }
            m_TsvInductance = std::stod(value) * std::pow(10, (magnitude_map[magnitude] - magnitude_map[stdMagnitude]));

        }else if(key == "C4_RADIUS"){
            if((stdUnit != unit) || (magnitude_map.find(magnitude) == magnitude_map.end())){
                std::cout << "[PowerX:TchParser] Unmatch unit for C4_RADIUS: " << magnitude << unit << std::endl;
                continue;
            }
            m_C4Radius = std::stod(value) * std::pow(10, (magnitude_map[magnitude] - magnitude_map[stdMagnitude]));

        }else if(key == "C4_RESISTANCE"){
            if((stdUnit != unit) || (magnitude_map.find(magnitude) == magnitude_map.end())){
                std::cout << "[PowerX:TchParser] Unmatch unit for C4_RESISTANCE: " << magnitude << unit << std::endl;
                continue;
            }
            m_C4Resistance = std::stod(value) * std::pow(10, (magnitude_map[magnitude] - magnitude_map[stdMagnitude]));

        }else if(key == "C4_INDUCTANCE"){
            if((stdUnit != unit) || (magnitude_map.find(magnitude) == magnitude_map.end())){
                std::cout << "[PowerX:TchParser] Unmatch unit for C4_INDUCTANCE: " << magnitude << unit << std::endl;
                continue;
            }
            m_C4Inductance = std::stod(value) * std::pow(10, (magnitude_map[magnitude] - magnitude_map[stdMagnitude]));

        }else if(key == "PCB_INDUCTANCE"){
            if((stdUnit != unit) || (magnitude_map.find(magnitude) == magnitude_map.end())){
                std::cout << "[PowerX:TchParser] Unmatch unit for PCB_INDUCTANCE: " << magnitude << unit << std::endl;
                continue;
            }
            m_PCBInductance = std::stod(value) * std::pow(10, (magnitude_map[magnitude] - magnitude_map[stdMagnitude]));

        }else if(key == "PCB_RESISTANCE"){
            if((stdUnit != unit) || (magnitude_map.find(magnitude) == magnitude_map.end())){
                std::cout << "[PowerX:TchParser] Unmatch unit for PCB_RESISTANCE: " << magnitude << unit << std::endl;
                continue;
            }
            m_PCBResistance = std::stod(value) * std::pow(10, (magnitude_map[magnitude] - magnitude_map[stdMagnitude]));

        }else if(key == "PCB_DECAP_INDUCTANCE"){
            if((stdUnit != unit) || (magnitude_map.find(magnitude) == magnitude_map.end())){
                std::cout << "[PowerX:TchParser] Unmatch unit for PCB_DECAP_INDUCTANCE: " << magnitude << unit << std::endl;
                continue;
            }
            m_PCBDecapInductance = std::stod(value) * std::pow(10, (magnitude_map[magnitude] - magnitude_map[stdMagnitude]));

        }else if(key == "PCB_DECAP_CAPACITANCE"){
            if((stdUnit != unit) || (magnitude_map.find(magnitude) == magnitude_map.end())){
                std::cout << "[PowerX:TchParser] Unmatch unit for PCB_DECAP_CAPACITANCE: " << magnitude << unit << std::endl;
                continue;
            }
            m_PCBDecapCapacitance = std::stod(value) * std::pow(10, (magnitude_map[magnitude] - magnitude_map[stdMagnitude]));

        }else if(key == "PCB_DECAP_RESISTANCE"){
            if((stdUnit != unit) || (magnitude_map.find(magnitude) == magnitude_map.end())){
                std::cout << "[PowerX:TchParser] Unmatch unit for PCB_DECAP_RESISTANCE: " << magnitude << unit << std::endl;
                continue;
            }
            m_PCBDecapResistance = std::stod(value) * std::pow(10, (magnitude_map[magnitude] - magnitude_map[stdMagnitude]));

        }else if(key == "DIE_METAL_RESISTIVITY"){
            if((stdUnit != unit) || (magnitude_map.find(magnitude) == magnitude_map.end())){
                std::cout << "[PowerX:TchParser] Unmatch unit for DIE_METAL_RESISTIVITY: " << magnitude << unit << std::endl;
                continue;
            }
            m_DieMetalResistivity = std::stod(value) * std::pow(10, (magnitude_map[magnitude] - magnitude_map[stdMagnitude]));

        }else if(key == "INTERPOSER_METAL_RESISTIVITY"){
            if((stdUnit != unit) || (magnitude_map.find(magnitude) == magnitude_map.end())){
                std::cout << "[PowerX:TchParser] Unmatch unit for INTERPOSER_METAL_RESISTIVITY: " << magnitude << unit << std::endl;
                continue;
            }
            m_InterposerMetalResistivity = std::stod(value) * std::pow(10, (magnitude_map[magnitude] - magnitude_map[stdMagnitude]));

        }else if(key == "PERMITIVITY_OF_FREE_SPACE"){
            if((stdUnit != unit) || (magnitude_map.find(magnitude) == magnitude_map.end())){
                std::cout << "[PowerX:TchParser] Unmatch unit for PERMITIVITY_OF_FREE_SPACE: " << magnitude << unit << std::endl;
                continue;
            }
            m_PermitivityOfFreeSpace = std::stod(value) * std::pow(10, (magnitude_map[magnitude] - magnitude_map[stdMagnitude]));

        }else if(key == "PERMITIVITY_OF_DIELECTRIC"){
            
            if(!unit.empty()){
                std::cout << "[PowerX:TchParser] Unmatch unit for PERMITIVITY_OF_DIELECTRIC: " << unit << std::endl;
                continue;
            }
            m_PermitivityOfDielectric = std::stod(value);

        }else if(key == "PERMEABILITY_OF_VACCUM"){
            if((stdUnit != unit) || (magnitude_map.find(magnitude) == magnitude_map.end())){
                std::cout << "[PowerX:TchParser] Unmatch unit for PERMEABILITY_OF_VACCUM: " << magnitude << unit << std::endl;
                continue;
            }
            m_PermeabilityOfVaccum = std::stod(value) * std::pow(10, (magnitude_map[magnitude] - magnitude_map[stdMagnitude]));

        }else if(key == "LOSS_TANGENT"){
            if(!unit.empty()){
                std::cout << "[PowerX:TchParser] Unmatch unit for LOSS_TANGENT: " << unit << std::endl;
                continue;
            }
            this->m_LossTangent = std::stod(value);

        }

    }

    file.close();

}