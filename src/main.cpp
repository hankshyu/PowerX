#include <iostream>
#include <assert.h>
#include <unordered_map>
#include "vector"

#include "boost/polygon/polygon.hpp"

// #include "isotropy.hpp"
// #include "rectangle.hpp"
// #include "tile.hpp"
// #include "rectilinear.hpp"

#include "line.hpp"

// #include "doughnutPolygon.hpp"

namespace gtl = boost::polygon;
using namespace boost::polygon::operators;

int main(int argc, char const *argv[]){
    
    Line l1(Cord(3, 5), Cord(3, 7));
    Line l2(Cord(3, 7), Cord(3, 5));

    std::cout << l1 << std::endl;
    std::cout << l2 << std::endl;

    Line l3 = l2;
    std::cout << l3 << std::endl;

    Line l4(Line(Cord(4, 5), Cord(9, 5)));
    std::cout << l4 << std::endl;

    std::cout << (l4 == l3) << std::endl;


    Line l5(Cord(4, 7), Cord(11, 14));

}
