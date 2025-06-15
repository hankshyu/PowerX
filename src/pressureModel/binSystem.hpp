//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        06/11/2025 15:58:38
//  Module Name:        binSystem.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A spatial location system that is based on binning
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
/////////////////////////////////////////////////////////////////////////////////

// Dependencies
// 1. C++ STL:
#include <vector>
// 2. Boost Library:

// 3. Texo Library:


#ifndef __BIN_SYSTEM_H__
#define __BIN_SYSTEM_H__

template<typename Scalar, typename T>
class BinSystem {
public:
    struct Entry {
        Scalar x, y;
        T* object;
    };
    
private:
    Scalar binSize;
    Scalar originX, originY;
    int gridWidth, gridHeight;

    std::vector<std::vector<Entry>> bins;

public:
    BinSystem(Scalar binSize, Scalar xmin, Scalar ymin, Scalar xmax, Scalar ymax)
        : binSize(binSize), originX(xmin), originY(ymin)
    {
        gridWidth  = static_cast<int>(std::ceil((xmax - xmin) / binSize));
        gridHeight = static_cast<int>(std::ceil((ymax - ymin) / binSize));
        bins.resize(gridWidth * gridHeight);
    }

    void clear() {
        for (auto& bin : bins) {
            bin.clear();
        }
    }

    void insert(Scalar x, Scalar y, T* obj) {
        int ix = static_cast<int>((x - originX) / binSize);
        int iy = static_cast<int>((y - originY) / binSize);
        if (ix >= 0 && ix < gridWidth && iy >= 0 && iy < gridHeight) {
            int binIndex = iy * gridWidth + ix;
            bins[binIndex].push_back({x, y, obj});
        }
    }

    // Query all points in a box (return Entry: includes (x, y, obj))
    std::vector<Entry> query(Scalar xmin, Scalar ymin, Scalar xmax, Scalar ymax) const {
        std::vector<Entry> results;
        int ix_min = static_cast<int>((xmin - originX) / binSize);
        int ix_max = static_cast<int>((xmax - originX) / binSize);
        int iy_min = static_cast<int>((ymin - originY) / binSize);
        int iy_max = static_cast<int>((ymax - originY) / binSize);

        for (int iy = iy_min; iy <= iy_max; ++iy) {
            for (int ix = ix_min; ix <= ix_max; ++ix) {
                if (ix >= 0 && ix < gridWidth && iy >= 0 && iy < gridHeight) {
                    int binIndex = iy * gridWidth + ix;
                    for (const auto& entry : bins[binIndex]) {
                        if (entry.x >= xmin && entry.x <= xmax &&
                            entry.y >= ymin && entry.y <= ymax) {
                            results.push_back(entry);
                        }
                    }
                }
            }
        }
        return results;
    }

    // Query with a user-defined filter function (polymorphic behavior)
    std::vector<Entry> query(
        Scalar xmin, Scalar ymin, Scalar xmax, Scalar ymax,
        std::function<bool(const T*, Scalar, Scalar)> predicate) const
    {
        std::vector<Entry> results;
        int ix_min = static_cast<int>((xmin - originX) / binSize);
        int ix_max = static_cast<int>((xmax - originX) / binSize);
        int iy_min = static_cast<int>((ymin - originY) / binSize);
        int iy_max = static_cast<int>((ymax - originY) / binSize);

        for (int iy = iy_min; iy <= iy_max; ++iy) {
            for (int ix = ix_min; ix <= ix_max; ++ix) {
                if (ix >= 0 && ix < gridWidth && iy >= 0 && iy < gridHeight) {
                    int binIndex = iy * gridWidth + ix;
                    for (const auto& entry : bins[binIndex]) {
                        if (entry.x >= xmin && entry.x <= xmax &&
                            entry.y >= ymin && entry.y <= ymax &&
                            predicate(entry.object, entry.x, entry.y)) {
                            results.push_back(entry);
                        }
                    }
                }
            }
        }
        return results;
    }

};

#endif // __BIN_SYSTEM_H__
