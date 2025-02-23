#include <iostream>
#include <assert.h>
#include <unordered_map>

#include "boost/polygon/polygon.hpp"
#include "interval.hpp"
#include "isotropy.hpp"


namespace gtl = boost::polygon;
using namespace boost::polygon::operators;

int main(int argc, char const *argv[]){



    // Interval iv1 = Interval(7, 3);
    // Interval *iv2 = new Interval(3, 7);
    // Interval iv3(*iv2);
    // Interval iv4(2, 17);
    
    // std::cout << iv1 << std::endl;
    // std::cout << iv2 << std::endl;
    // std::cout << iv3 << std::endl;
    // std::cout << iv4 << std::endl;

    
    // std::cout << boost::polygon::delta(iv4) << std::endl;
    
    Interval it1(9, 14);
    
    Interval it2(10, 13);
    Interval it3(3, 9);
    Interval it4(3, 12);

    std::cout << boost::polygon::intersects(it1, it2, true);
    std::cout << boost::polygon::intersects(it1, it3, true);
    std::cout << boost::polygon::intersects(it1, it4, true);

    std::cout << boost::polygon::boundaries_intersect(it1, it2, true);
    std::cout << boost::polygon::boundaries_intersect(it1, it3, true);
    std::cout << boost::polygon::boundaries_intersect(it1, it4, true);


    return 0;
}
