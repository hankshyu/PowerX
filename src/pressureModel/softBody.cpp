//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        06/07/2025 17:12:24
//  Module Name:        softBody.cpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        The object that expands, shrink and merge due to
//                      pressure-driven forces
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
/////////////////////////////////////////////////////////////////////////////////


// Dependencies
// 1. C++ STL:
#include <ostream>

// 2. Boost Library:
#include "boost/geometry.hpp"
#include <boost/math/constants/constants.hpp>

// 3. Texo Library:
#include "units.hpp"
#include "softBody.hpp"




// Constructor
SoftBody::SoftBody(int id, SignalType sig, double expectCurrent, farea_t initArea):
    m_id(id), m_sigType(sig), m_expectCurrent(expectCurrent), m_initialArea(initArea){
    
    // TODO: calculate initial pressure
}
// Getter for signal type
SignalType SoftBody::getSigType() const {
    return m_sigType;
}

// Getter for expected current
double SoftBody::getExpectCurrent() const {
    return m_expectCurrent;
}

double SoftBody::calculatePressure() const{
    FPolygon fpcontour = fp::createFPolygon(contour);
    return m_expectCurrent * (fp::getArea(fpcontour)/m_initialArea);
}

void SoftBody::remeshContour(flen_t minDelta){

    size_t rawContourSize = contour.size();
    if(rawContourSize < 4) return;
    
    // Estimate L2 using L1
    
    constexpr double sqrt2 = boost::math::constants::root_two<double>();

    const flen_t L2_MIN_DELTA = minDelta * sqrt2;
    const flen_t USE_CURVATURE_DELTA = 4 * L2_MIN_DELTA;


    std::vector<FPoint> remeshedContour;

    auto remeshBetweenPoints = [&](const FPoint &P, const FPoint &A, const FPoint &B, const FPoint &Q){
        remeshedContour.emplace_back(A);
        flen_t L2RawDistance = calManhattanDistance(A, B);
        if(L2RawDistance > L2_MIN_DELTA){
            if(L2RawDistance > USE_CURVATURE_DELTA){

                // Arch Estimation: L \approx \|P_3 - P_0\| + \frac{1}{3} \left( \|P_1 - P_0\| + \|P_2 - P_3\| \right)
                // Floater, M. Arc Length Estimation and the Convergence of Polynomial Curve Interpolation. 
                // Bit Numer Math 45, 679–694 (2005). https://doi.org/10.1007/s10543-005-0031-2
                
                flen_t px = P.x(), py = P.y();
                flen_t ax = A.x(), ay = A.y();
                flen_t bx = B.x(), by = B.y();
                flen_t qx = Q.x(), qy = Q.y();

                // Tangents (centered difference)
                flen_t tax = (bx - px) / 2.0;
                flen_t tay = (by - py) / 2.0;
                flen_t tbx = (qx - ax) / 2.0;
                flen_t tby = (qy - ay) / 2.0;

                // Bezier control points A , p1, p2, B

                flen_t p1x = ax + tax / 3.0;
                flen_t p1y = ay + tay / 3.0;
                flen_t p2x = bx - tbx / 3.0;
                flen_t p2y = by - tby / 3.0;

                // Kim & Kim arc length approximation
                flen_t chord = std::hypot(bx - ax, by - ay);
                flen_t ctrl1 = std::hypot(p1x - ax, p1y - ay);
                flen_t ctrl2 = std::hypot(p2x - bx, p2y - by);

                chord += (ctrl1 + ctrl2) / 3.0;
                
                int newPoints = static_cast<int>(chord/minDelta) + 1;
                flen_t step = 1.0 / (newPoints + 1);
                
                flen_t t = step;
                // Generate evenly spaced t values
                for (int i = 0; i <newPoints; ++i) {
                    flen_t u = 1.0 - t;
                    
                    // reuse multiplication results
                    flen_t threeu2t1 = u * u;
                    flen_t u3 = threeu2t1 * u;
                    threeu2t1 = 3 * threeu2t1 * t;
                    flen_t threeu1t2 = t*t;
                    flen_t t3 = threeu1t2 * t;
                    threeu1t2 *= 3 * threeu1t2 * u;

                    // B(t) = (1 - t)^3 P_0 + 3(1 - t)^2 t P_1 + 3(1 - t) t^2 P_2 + t^3 P_3
                    flen_t x = u3 * ax + threeu2t1 * p1x + threeu1t2 * p2x + t3 * bx;
                    flen_t y = u3 * ay + threeu2t1 * p1y + threeu1t2 * p2y + t3 * by;
                    
                    remeshedContour.emplace_back(x, y);
                    t += step;
                }
            }else{

                flen_t ax = A.x(), ay = A.y();
                flen_t bx = B.x(), by = B.y();

                // use Linear interpolation to mesh
                flen_t dx = bx - ax;
                flen_t dy = by - ay;
                flen_t distance = std::hypot(dx, dy);
                
                int newPoints = int(distance/minDelta) + 1;
                int newSegments = newPoints + 1; // 1 points -> 2 segments

                dx = dx / newSegments;
                dy = dy / newSegments;

                // FPoint insertPoint(contour[0]);

                for(int i = 0; i < newPoints; ++i){
                    ax += dx;
                    ay += dy;
                    remeshedContour.emplace_back(ax, ay);
                }
            }
        }
    };

    remeshBetweenPoints(contour[rawContourSize-1], contour[0], contour[1], contour[2]);
    
    for(int i = 1; i <= rawContourSize-3; ++i){
        remeshBetweenPoints(contour[i-1], contour[i], contour[i+1], contour[i+2]);
    }
    remeshBetweenPoints(contour[rawContourSize-3], contour[rawContourSize-2], contour[rawContourSize-1], contour[0]);
    remeshBetweenPoints(contour[rawContourSize-2], contour[rawContourSize-1], contour[0], contour[1]);
}
