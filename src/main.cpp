#include <iostream>
#include <assert.h>
#include <unordered_map>
#include "vector"

#include "boost/polygon/polygon.hpp"

#include "rectangle.hpp"
#include "doughnutPolygon.hpp"
#include "doughnutPolygonSet.hpp"

// #include "doughnutPolygon.hpp"

namespace gtl = boost::polygon;
using namespace boost::polygon::operators;

int main(int argc, char const *argv[]){

    DoughnutPolygonSet dps;
    DoughnutPolygon dp;

    dps += Rectangle(1, 12, 4, 17);
    dps += Rectangle(4, 11, 8, 13);
    dps += Rectangle(4, 15, 12, 19);
    dps += Rectangle(8, 12, 10, 15);
    dps += Rectangle(10, 11, 12, 13);
    dps += Rectangle(12, 13, 13, 15);
    dps += Rectangle(14, 17, 15, 19);


    std::cout << dps::getArea(dps) << std::endl;
    std::cout << dps.size() << std::endl;
    dp = dps[1];
    std::cout << dp.size_holes() << std::endl;


    // std::cout << dps::getArea(dps) << std::endl;
    // std::cout << dps::getBoundingBox(dps) << std::endl;
    // std::cout << dps::getShapesCount(dps) << std::endl;
    
    std::vector<Rectangle> frag;
    // dps::diceIntoRectangles(dps, frag);
    // dou
    dps::diceIntoRectangles(dps, frag, eOrientation2D::HORIZONTAL);


    for(const Rectangle &rec : frag){
        std::cout << rec << std::endl;
    }

    dp = dps[0];

    std::cout << dp::getEdgeCount(dp) << std::endl;
    std::cout << dp::getPerimeter(dp) << std::endl;
    std::cout << dp::getArea(dp) << std::endl;
    std::cout << dp::getBoundingBox(dp) << std::endl;
    std::cout << dp << std::endl;

}
