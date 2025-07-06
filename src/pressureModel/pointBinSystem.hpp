//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        06/11/2025 15:58:38
//  Module Name:        pointBinSystem.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A spatial location system of points that is based on binning
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//  06/17/2025          Change Class name, change return and internal Entry type
/////////////////////////////////////////////////////////////////////////////////

// Dependencies
// 1. C++ STL:
#include <vector>
#include <cmath>
// 2. Boost Library:

// 3. Texo Library:


#ifndef __POINT_BIN_SYSTEM_H__
#define __POINT_BIN_SYSTEM_H__

template<typename Scalar, typename T>
struct PointEntry{
    Scalar x;
    Scalar y;
    T *object;

    PointEntry() = default;

    PointEntry(Scalar x, Scalar y, T *object)
        : x(x), y(y), object(object) {}


};

template<typename Scalar, typename T>
struct PointDistanceEntry{
    Scalar x;
    Scalar y;
    Scalar distance;
    T* object;


    PointDistanceEntry() = default;

    PointDistanceEntry(Scalar x, Scalar y, Scalar distance, T *object)
        : x(x), y(y), distance(distance), object(object) {}
};

template<typename Scalar, typename T>
class PointBinSystem{
private:
    using Entry = PointEntry<Scalar, T>;
    
    Scalar binSize;
    Scalar originX, originY;
    int gridWidth, gridHeight;

    Scalar canvasWidth, canvasHeight;

    std::vector<std::vector<Entry>> bins;

public:
    PointBinSystem(Scalar binSize, Scalar xmin, Scalar ymin, Scalar xmax, Scalar ymax)
        : binSize(binSize), originX(xmin), originY(ymin), canvasWidth(xmax - xmin), canvasHeight(ymax - ymin) {
        
        gridWidth  = static_cast<int>(std::ceil((xmax - xmin) / binSize));
        gridHeight = static_cast<int>(std::ceil((ymax - ymin) / binSize));

        bins.resize(gridWidth * gridHeight);
    }

    void insert(Scalar x, Scalar y, T* obj) {
        int ix = static_cast<int>(std::floor((x - originX) / binSize));
        int iy = static_cast<int>(std::floor((y - originY) / binSize));

        if (ix < 0 || ix >= gridWidth || iy < 0 || iy >= gridHeight) return;
        
        int binIndex = iy * gridWidth + ix;
        bins[binIndex].push_back({x, y, obj});
    }

    // return if success (entry found)
    bool remove(Scalar x, Scalar y, T* obj) {
        int ix = static_cast<int>(std::floor((x - originX) / binSize));
        int iy = static_cast<int>(std::floor((y - originY) / binSize));

        if (ix < 0 || ix >= gridWidth || iy < 0 || iy >= gridHeight) return false;

        int binIndex = iy * gridWidth + ix;
        auto& bin = bins[binIndex];
        for(int i = 0; i < bin.size(); ++i){
            const Entry &pe = bin[i];
            if(pe.x == x && pe.y == y && pe.object == obj){
                bin.erase(bin.begin() + i);
                return true;
            }
        }
        return false;
    }

    // return if success (at least one entry found)
    bool remove(Scalar x, Scalar y) {
        int ix = static_cast<int>(std::floor((x - originX) / binSize));
        int iy = static_cast<int>(std::floor((y - originY) / binSize));

        if (ix < 0 || ix >= gridWidth || iy < 0 || iy >= gridHeight) return false;

        int binIndex = iy * gridWidth + ix;
        auto& bin = bins[binIndex];

        size_t binSize = bin.size();
        
        size_t writeIdx = 0;
        bool entryRemoved = false;

        // remove in-place
        for (size_t readIdx = 0; readIdx < binSize; ++readIdx) {
            Entry& pe = bin[readIdx];
            if (pe.x == x && pe.y == y) {
                entryRemoved = true;
                continue;
            }
            bin[writeIdx++] = std::move(pe);
        }
        bin.resize(writeIdx);
        return entryRemoved;
    }

    // remove all entries from the bin system
    void clear() noexcept {
        for (auto& bin : bins) {
            bin.clear();
        }
    }

    // Query all points in a box (return Entry: includes (x, y, obj))
    std::vector<PointEntry<Scalar, T>> query(Scalar xmin, Scalar ymin, Scalar xmax, Scalar ymax) const {
        std::vector<Entry> results;
        int ix_min = static_cast<int>((xmin - originX) / binSize);
        int ix_max = static_cast<int>((xmax - originX) / binSize);
        int iy_min = static_cast<int>((ymin - originY) / binSize);
        int iy_max = static_cast<int>((ymax - originY) / binSize);

        ix_min = std::max(0, ix_min);
        iy_min = std::max(0, iy_min);
        ix_max = std::min(gridWidth - 1, ix_max);
        iy_max = std::min(gridHeight - 1, iy_max);

        for (int iy = iy_min; iy <= iy_max; ++iy) {
            for (int ix = ix_min; ix <= ix_max; ++ix) {
                int binIndex = iy * gridWidth + ix;
                for (const Entry& entry : bins[binIndex]) {
                    if (entry.x >= xmin && entry.x <= xmax && entry.y >= ymin && entry.y <= ymax) {
                        results.push_back(entry);
                    }
                }
            }
        }

        return results;
    }

    std::vector<PointDistanceEntry<Scalar, T>> queryDistance(Scalar xCentre, Scalar yCentre, Scalar distance) const {
        std::vector<PointDistanceEntry<Scalar, T>> results;

        Scalar distanceSq = distance * distance;

        // First, compute the bounding square around the circle
        Scalar xmin = xCentre - distance;
        Scalar ymin = yCentre - distance;
        Scalar xmax = xCentre + distance;
        Scalar ymax = yCentre + distance;

        int ix_min = static_cast<int>((xmin - originX) / binSize);
        int iy_min = static_cast<int>((ymin - originY) / binSize);
        int ix_max = static_cast<int>((xmax - originX) / binSize);
        int iy_max = static_cast<int>((ymax - originY) / binSize);

        ix_min = std::max(0, ix_min);
        iy_min = std::max(0, iy_min);
        ix_max = std::min(gridWidth - 1, ix_max);
        iy_max = std::min(gridHeight - 1, iy_max);

        for (int iy = iy_min; iy <= iy_max; ++iy) {
            for (int ix = ix_min; ix <= ix_max; ++ix) {
                int binIndex = iy * gridWidth + ix;
                for (const Entry& entry : bins[binIndex]) {
                    Scalar dx = entry.x - xCentre;
                    Scalar dy = entry.y - yCentre;
                    Scalar distSq = dx * dx + dy * dy;
                    if (distSq < distanceSq) {
                        results.emplace_back(entry.x, entry.y, sqrt(distSq), entry.object);
                    }
                }
            }
        }

        return results;
    }


    PointDistanceEntry<Scalar, T> queryNearestNeighbor(Scalar x, Scalar y) const {
        Scalar searchRadius = binSize;
        PointDistanceEntry<Scalar, T> nearest = {x, y, std::numeric_limits<Scalar>::max(), nullptr};

        // Max possible distance: diagonal of the canvas
        const Scalar maxRadius = std::hypot(canvasWidth, canvasHeight);

        while (searchRadius <= maxRadius) {
            auto candidates = queryDistance(x, y, searchRadius);

            for (const auto& entry : candidates) {
                if (entry.distance != 0 && entry.distance < nearest.distance) {
                    nearest.x = entry.x;
                    nearest.y = entry.y;
                    nearest.distance = entry.distance;
                    nearest.object = entry.object;
                }
            }

            if (nearest.object != nullptr) break;
            searchRadius *= 2;
        }

        return nearest;
    }
};




#endif // __POINT_BIN_SYSTEM_H__