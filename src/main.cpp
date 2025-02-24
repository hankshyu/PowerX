#include <iostream>
#include <assert.h>
#include <unordered_map>

#include "boost/polygon/polygon.hpp"

#include "isotropy.hpp"
#include "cord.hpp"
#include "fcord.hpp"
#include "line.hpp"
#include "interval.hpp"
#include "rectangle.hpp"


namespace gtl = boost::polygon;
using namespace boost::polygon::operators;

int main(int argc, char const *argv[]){

    Rectangle r1(0, 3, 5, 11);
    std::cout << "r1 = " << r1 << std::endl;

    std::cout << r1.getXL() << std::endl;
    std::cout << r1.getYL() << std::endl;
    std::cout << r1.getXH() << std::endl;
    std::cout << r1.getYH() << std::endl;

    std::cout << r1.getLL() << std::endl;
    std::cout << r1.getLR() << std::endl;
    std::cout << r1.getUL() << std::endl;
    std::cout << r1.getUR() << std::endl;
    
    FCord ct1;
    boost::polygon::center(ct1, r1);
    std::cout << ct1 << std::endl;

    std::cout << r1.getCentre() << std::endl;

    std::cout << boost::polygon::contains(r1, Cord(5, 3), false) << std::endl; 
    return 0;
}
