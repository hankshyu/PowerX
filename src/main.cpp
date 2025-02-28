#include <iostream>
#include <assert.h>
#include <unordered_map>
#include "vector"

#include "boost/polygon/polygon.hpp"
#include "fcord.hpp"
#include "rectangle.hpp"

// #include "doughnutPolygon.hpp"

namespace gtl = boost::polygon;
using namespace boost::polygon::operators;

int main(int argc, char const *argv[])
{
    Rectangle rec1(3, 7, 6, 9);
    std::cout << rec::getLL(rec1) << std::endl;
    std::cout << rec::getLR(rec1) << std::endl;
    std::cout << rec::getUL(rec1) << std::endl;

    std::cout << rec::hasIntersect(rec1, Rectangle(6, 9, 7, 11)) << std::endl;
    std::cout << rec::calculateCentre(rec1);

}
