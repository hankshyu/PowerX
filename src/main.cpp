#include <iostream>
#include <assert.h>
#include <unordered_map>

#include "boost/polygon/polygon.hpp"
#include "interval.hpp"


namespace gtl = boost::polygon;
using namespace boost::polygon::operators;

int main(int argc, char const *argv[]){

    boost::polygon::direction_1d d1 = boost::polygon::direction_1d_enum::LOW;

    std::cout << boost::polygon::direction_1d_enum::LOW << std::endl;


    // Interval iv1 = Interval(7, 3);
    // Interval *iv2 = new Interval(3, 7);
    // Interval iv3(*iv2);
    // Interval iv4(2, 17);
    
    // std::cout << iv1 << std::endl;
    // std::cout << iv2 << std::endl;
    // std::cout << iv3 << std::endl;
    // std::cout << iv4 << std::endl;

    
    // std::cout << boost::polygon::delta(iv4) << std::endl;
    
    



    return 0;
}
