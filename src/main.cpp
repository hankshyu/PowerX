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
#include "tile.hpp"
#include "lineTile.hpp"

// #include "doughnutPolygon.hpp"

namespace gtl = boost::polygon;
using namespace boost::polygon::operators;

int main(int argc, char const *argv[]){
    
    Tile *t1 = new Tile(tileType::BLOCK, Rectangle(3, 4, 11, 12));
    Line l1(Cord(3, 12), Cord(5, 12));

    LineTile lt(l1, t1);
    std::cout << l1 << std::endl;
    std::cout << *t1 << std::endl;

    std::cout << lt << std::endl;


}
