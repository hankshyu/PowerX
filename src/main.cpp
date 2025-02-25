#include <iostream>
#include <assert.h>
#include <unordered_map>
#include "vector"

#include "boost/polygon/polygon.hpp"

#include "isotropy.hpp"
#include "cord.hpp"
#include "fcord.hpp"
#include "line.hpp"
#include "interval.hpp"
#include "rectangle.hpp"
#include "tile.hpp"




namespace gtl = boost::polygon;
using namespace boost::polygon::operators;

int main(int argc, char const *argv[]){

    Rectangle r1 (0, 2, 7, 9);
    Rectangle r2 (7, 9, 11, 13);

    Tile t1(0, 2, 7, 9);
    Tile t2(7, 9, 11, 13);

    std::cout << t1.bl << t1.lb << std::endl;

    std::cout << boost::polygon::intersects(r1, r2, false) << std::endl;
    std::cout << boost::polygon::intersects(t1, t2, false) << std::endl;
    std::cout << boost::polygon::intersects(r1, t2, false) << std::endl;
    // std::cout << boost::polygon::contains(t1, Cord(1, 7), true) << std::endl;
    return 0;
}
