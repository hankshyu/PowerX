//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        06/18/2025 18:01:35
//  Module Name:        rectangleBinSystem.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A spatial location system of rectangles that is based on
//                      binning. Written as a template
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
/////////////////////////////////////////////////////////////////////////////////

// Dependencies
// 1. C++ STL:
#include <vector>
#include <cmath>
#include <cstdint>
#include <cassert>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <utility>

// 2. Boost Library:

// 3. Texo Library:

#ifndef __RECTANGLE_BIN_SYSTEM_H__
#define __RECTANGLE_BIN_SYSTEM_H__

template<typename Scalar, typename T>
struct RectangleEntry {
    Scalar xmin, ymin, xmax, ymax;
    T *object;
};


template <typename Scalar, typename T>
class RectangleBinSystem {
private:
    using Entry = RectangleEntry<Scalar, T>;
    
    Scalar m_binSize;
    Scalar m_canvasMinX, m_canvasMaxX;
    Scalar m_canvasMinY, m_canvasMaxY;

    int m_gridWidth, m_gridHeight;

    std::vector<std::vector<Entry>> m_bins; // a flattened 1D array to store Entries
    std::unordered_map<T*, std::vector<int>> m_rectToBins;

    int clamp(int v, int max) const {
        return std::max(0, std::min(v, max - 1));
    }

    int binIndex(int x, int y) const {
        return y * m_gridWidth + x;
    }

    bool outOfBounds(Scalar xmin, Scalar ymin, Scalar xmax, Scalar ymax) const {
        return (xmin < m_canvasMinX) || (ymin < m_canvasMinY) ||
               (xmax > m_canvasMaxX) || (ymax > m_canvasMaxY);
    }

    std::vector<int> getCoveredBinIndices(Scalar xmin, Scalar ymin, Scalar xmax, Scalar ymax) const {
        int x0 = clamp(static_cast<int>((xmin - m_canvasMinX) / m_binSize), m_gridWidth);
        int x1 = clamp(static_cast<int>((xmax - m_canvasMinX) / m_binSize), m_gridWidth);
        int y0 = clamp(static_cast<int>((ymin - m_canvasMinY) / m_binSize), m_gridHeight);
        int y1 = clamp(static_cast<int>((ymax - m_canvasMinY) / m_binSize), m_gridHeight);

        std::vector<int> indices;
        for (int x = x0; x <= x1; ++x)
            for (int y = y0; y <= y1; ++y)
                indices.push_back(binIndex(x, y));
        return indices;
    }

public:

    RectangleBinSystem(Scalar binSize, Scalar xmin, Scalar ymin, Scalar xmax, Scalar ymax)
        : m_binSize(binSize), m_canvasMinX(xmin), m_canvasMinY(ymin), m_canvasMaxX(xmax), m_canvasMaxY(ymax) {
        
        m_gridWidth = static_cast<int>(std::ceil((m_canvasMaxX - m_canvasMinX) / m_binSize));
        m_gridHeight = static_cast<int>(std::ceil((m_canvasMaxY - m_canvasMinY) / m_binSize));
        m_bins.resize(m_gridWidth * m_gridHeight);
    }
    
    // return true if inserted, false if duplicate or out-of-bounds
    bool insert(Scalar xmin, Scalar ymin, Scalar xmax, Scalar ymax, T* obj) {
        if (m_rectToBins.count(obj)) return false;
        if (outOfBounds(xmin, ymin, xmax, ymax)) return false;
        if ((xmin > xmax) || (ymin > ymax)) return false;

        Entry entry = {xmin, ymin, xmax, ymax, obj};
        std::vector<int> indices = getCoveredBinIndices(xmin, ymin, xmax, ymax);

        for (int idx : indices){
            m_bins[idx].emplace_back(entry);
        }

        m_rectToBins[obj] = std::move(indices);
        return true;
    }
    
    bool remove(T* obj) {
        auto it = m_rectToBins.find(obj);
        if (it == m_rectToBins.end()) return false;

        for (int idx : it->second) {
            auto& vec = m_bins[idx];
            //erase all T* present
            vec.erase(std::remove_if(vec.begin(), vec.end(),
                [obj](const Entry& e) { return e.object == obj; }), vec.end());
        }

        m_rectToBins.erase(it);
        return true;
    }

    // consier toucing the rectangle even on the border of the rectangle
    std::vector<T*> queryPoint(Scalar x, Scalar y) const {
        if (x < m_canvasMinX || y < m_canvasMinY || x > m_canvasMaxX || y > m_canvasMaxY)
            return {};

        int xIdx = clamp(static_cast<int>((x - m_canvasMinX) / m_binSize), m_gridWidth);
        int yIdx = clamp(static_cast<int>((y - m_canvasMinY) / m_binSize), m_gridHeight);
        int idx = binIndex(xIdx, yIdx);

        std::vector<T*> result;
        for (const auto& e : m_bins[idx]) {
            if (x >= e.xmin && x <= e.xmax &&
                y >= e.ymin && y <= e.ymax) {
                result.push_back(e.object);
            }
        }
        return result;
    }

    // will not report touching rectangles(edge, border)
    std::vector<std::pair<T*, T*>> reportOverlap() const {
        std::set<std::pair<T*, T*>> uniquePairs;

        for (const auto& bin : m_bins) {
            const std::size_t n = bin.size();
            for (std::size_t i = 0; i < n; ++i) {
                const Entry& a = bin[i];
                for (std::size_t j = i + 1; j < n; ++j) {
                    const Entry& b = bin[j];
                    if (a.object == b.object) continue;

                    if (a.xmax > b.xmin && a.xmin < b.xmax && a.ymax > b.ymin && a.ymin < b.ymax) {

                        T* aPtr = a.object;
                        T* bPtr = b.object;
                        if (aPtr > bPtr) std::swap(aPtr, bPtr); // enforce order

                        uniquePairs.emplace(aPtr, bPtr);
                    }
                }
            }
        }

        return {uniquePairs.begin(), uniquePairs.end()};
    }

    bool contains(T* obj) const {
        return m_rectToBins.count(obj);
    }

    void clear() {
        for (auto& bin : m_bins){
            bin.clear();
        }
        
        m_rectToBins.clear();
    }

};


#endif // __RECTANGLE_BIN_SYSTEM_H__