#include <iostream>
#include <assert.h>
#include <unordered_map>

#include "boost/polygon/polygon.hpp"

#include "cord.hpp"
#include "line.hpp"


namespace gtl = boost::polygon;
using namespace boost::polygon::operators;

int main(int argc, char const *argv[]){

    Line l1(Cord(7, 9), Cord(2, 5));
    Line *l2 = new Line(l1);
    std::cout << l1 << std::endl;
    std::cout << (l1 == *l2) << std::endl;


    return 0;
}
