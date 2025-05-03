//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        03/26/2025 22:16:46
//  Module Name:        objectArray.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A data structure that describes an array of pins, will be
//                      use to hold cross-layer pin array, micro-Bumps and C4 Bumps
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//  03/28/2025          Remove 2D grid data structure
//  03/29/2025          Remove Signal Type ID counting mechanics, use SignalType
//  05/02/2025          Resort to Array storage, move unordered mappings to child classes
//
/////////////////////////////////////////////////////////////////////////////////

#ifndef __OBJECT_ARRAY_H__
#define __OBJECT_ARRAY_H__

// Dependencies
// 1. C++ STL:
#include <string>
#include <vector>


// 2. Boost Library:

// 3. Texo Library:
#include "cord.hpp"
#include "signalType.hpp"
#include "technology.hpp"

class ObjectArray{
protected:
    int m_width;
    int m_height;
    
public:
    std::vector<std::vector<SignalType>> canvas;
    std::unordered_map<SignalType, std::vector<Cord>> preplacedCords;
    
    ObjectArray();
    explicit ObjectArray(int width, int height);
    
    void readBlockages(const std::string &fileName);

    inline int getWidth() const {return this->m_width;}
    inline int getHeight() const {return this->m_height;}
    
    inline void setCanvas(const Cord &pos, SignalType sig) {this->canvas[pos.y()][pos.x()] = sig;}
    void setCanvas(int j, int i, SignalType sig) {this->canvas[j][i] = sig;}
    
    void markPreplacedToCanvas();

    friend bool visualisePinArray(const std::vector<std::vector<SignalType>> &pinArr, const Technology &tch, const std::string &filePath);
    friend bool visualiseGridArray(const std::vector<std::vector<SignalType>> &gridArr, const Technology &tch, const std::string &filePath);
    friend bool visualiseGridArrayWithPin(const std::vector<std::vector<SignalType>> &gridArr, const std::vector<std::vector<SignalType>> &pinArr, const Technology &tch, const std::string &filePath);
    friend bool visualiseGridArrayWithPins(const std::vector<std::vector<SignalType>> &gridArr,const std::vector<std::vector<SignalType>> &upPinArr, const std::vector<std::vector<SignalType>> &downPinArr, const Technology &tch, const std::string &filePath);

};

#endif // __OBJECT_ARRAY_H__
