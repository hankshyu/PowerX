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
#include <fstream>
#include <chrono>
#include <ctime>
#include <iomanip>

// 2. Boost Library:

// 3. Texo Library:
#include "technology.hpp"
#include "eqCktExtractor.hpp"
#include "powerGrid.hpp"


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

void EqCktExtractor::exportEquivalentCircuit(const PowerGrid &pg, const SignalType &st, const std::string &filePath) const {
    
    std::ofstream ofs(filePath, std::ios::out);
    assert(ofs.is_open());

    ofs << "* Equivalent Circuit Model of Extracted from PowerX" << std::endl;
    ofs << "* Model Name: " << pg.uBump.getName() << std::endl;
    
    // include the date time in comment
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm = *std::localtime(&now_time_t);
    ofs << "* Date/Time: " << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S") << std::endl;
    ofs << std::endl;
    ofs << "*******************************************************************************" << std::endl;
    ofs << "* 1. PCB Subcircuit" << std::endl;
    ofs << "*******************************************************************************" << std::endl;
    ofs << ".subckt pcb vrm_i, vrm_o, pcb_o, pcb_i" << std::endl;
    ofs << "Lpcbh vrm_i N1PCB " << this->m_Technology.getPCBInductance() << "p" << std::endl;
    ofs << "Rpcbh N1PCB pcb_o " << this->m_Technology.getPCBResistance() << "u" << std::endl;
    ofs << "Lcappcb pcb_o N2PCB " << this->m_Technology.getPCBDecapInductance() << "n" << std::endl;
    ofs << "Ccappcb N2PCB N3PCB " << this->m_Technology.getPCBDecapCapacitance() << "u" << std::endl;
    ofs << "Rcappcb N3PCB pcb_i " << this->m_Technology.getPCBDecapResistance() << "u" << std::endl;
    ofs << "Lpcbl pcb_i N4PCB " << this->m_Technology.getPCBInductance() << "p" << std::endl;
    ofs << "Rpcbl N4PCB vrm_o " << this->m_Technology.getPCBResistance() << "u" << std::endl;
    ofs << ".ends pcb" << std::endl;

    ofs << std::endl << std::endl;


    ofs << "*******************************************************************************" << std::endl;
    ofs << "* 2. C4 with RL modeling" << std::endl;
    ofs << "*******************************************************************************" << std::endl;
    ofs << ".subckt c4 in out" << std::endl;
    ofs << "Rc4 in N1C4 " <<  this->m_Technology.getC4Resistance() << "m" << std::endl;
    ofs << "Lc4 N1C4 out " << this->m_Technology.getC4Inductance() << "p" << std::endl;
    ofs << ".ends c4" << std::endl;

    ofs << std::endl << std::endl;

    ofs << "*******************************************************************************" << std::endl;
    ofs << "* 3. TSV with RL modeling" << std::endl;
    ofs << "*******************************************************************************" << std::endl;
    ofs << ".subckt tsv in out" << std::endl;
    ofs << "Rtsv in N1TSV " <<  this->m_Technology.getTsvResistance() << "m" << std::endl;
    ofs << "Ltsv N1TSV out " << this->m_Technology.getTsvInductance() << "p" << std::endl;
    ofs << ".ends tsv" << std::endl;

    ofs << std::endl << std::endl;

    ofs << "*******************************************************************************" << std::endl;
    ofs << "* 4. MicroBump with RL modeling " << std::endl;
    ofs << "*******************************************************************************" << std::endl;
    ofs << ".subckt ubump in out" << std::endl;
    ofs << "Rubump in N1UB " << this->m_Technology.getMicrobumpResistance() << "m" << std::endl;
    ofs << "Lubump N1UB out " << this->m_Technology.getMicrobumpInductance() << "p" << std::endl;
    ofs << ".ends ubump" << std::endl;

    ofs << std::endl << std::endl;

    ofs << "*******************************************************************************" << std::endl;
    ofs << "* Instantiate main models, create connections to Interposer Equivalent Circuits" << std::endl;
    ofs << "*******************************************************************************" << std::endl;
    ofs << "* TODO " << std::endl;

    ofs << std::endl << std::endl;


    ofs << "*******************************************************************************" << std::endl;
    ofs << "* Construct the Interposer Equivalent Circuits" << std::endl;
    ofs << "*******************************************************************************" << std::endl;
    // builing the equivalent circuit of the interposer
    double itpR = 2 * getInterposerResistance();
    double itpL = 2 * getInterposerInductance();

    double itpViaR = getInterposerViaResistance();
    double itpViaL = getInterposerViaInductance();

    for(int j = 0; j < pg.canvasHeight; ++j){
        for(int i = 0; i < pg.canvasWidth; ++i){
            bool m5hit = pg.canvasM5[j][i] == st;
            bool m7hit = pg.canvasM7[j][i] == st;
            std::string m5node = nodeToString(5, i, j);
            std::string m7node = nodeToString(7, i, j);
            if(m5hit){
                if(j > 0){
                    if(pg.canvasM5[j-1][i] == st){
                        std::string downNode = nodeToString(5, i, j-1);
                        std::string tmpNode = downNode + "_" + m5node;
                        ofs << "R" + tmpNode << " " << "N" + downNode << " " << "N" + tmpNode << " " << itpR << "m" << std::endl;
                        ofs << "L" + tmpNode << " " << "N" + tmpNode << " " << "N" + m5node << " " << itpL << "p" << std::endl;
                    }
                }
                if(i > 0){
                    if(pg.canvasM5[j][i-1] == st){
                        std::string leftNode = nodeToString(5, i-1, j);
                        std::string tmpNode = leftNode + "_" + m5node;
                        ofs << "R" + tmpNode << " " << "N" + leftNode << " " << "N" + tmpNode << " " << itpR << "m" << std::endl;
                        ofs << "L" + tmpNode << " " << "N" + tmpNode << " " << "N" + m5node << " " << itpL << "p" << std::endl;
                    }
                }
            }
            if(m7hit){
                if(j > 0){
                    if(pg.canvasM7[j-1][i] == st){
                        std::string downNode = nodeToString(7, i, j-1);
                        std::string tmpNode = downNode + "_" + m7node;
                        ofs << "R" + tmpNode << " " << "N" + downNode << " " << "N" + tmpNode << " " << itpR << "m" << std::endl;
                        ofs << "L" + tmpNode << " " << "N" + tmpNode << " " << "N" + m7node << " " << itpL << "p" << std::endl;
                    }
                }
                if(i > 0){
                    if(pg.canvasM7[j][i-1] == st){
                        std::string leftNode = nodeToString(7, i-1, j);
                        std::string tmpNode = leftNode + "_" + m7node;
                        ofs << "R" + tmpNode << " " << "N" + leftNode << " " << "N" + tmpNode << " " << itpR << "m" << std::endl;
                        ofs << "L" + tmpNode << " " << "N" + tmpNode << " " << "N" + m7node << " " << itpL << "p" << std::endl;
                    }
                }
            }
            if(m5hit && m7hit){
                //connect Vias
                std::string tmpNode = m5node + "_" + m7node;
                ofs << "RT" + tmpNode << " " << "N" + m5node << " " << "N" + tmpNode << " " << itpViaR << "m" << std::endl;
                ofs << "LT" + tmpNode << " " << "N" + tmpNode << " " << "N" + m7node << " " << itpViaL << "p" << std::endl;
            }
        }
    }


}
