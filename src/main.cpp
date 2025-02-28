#include <iostream>
#include <assert.h>
#include <unordered_map>
#include "vector"

#include "boost/polygon/polygon.hpp"

#include "cord.hpp"
#include "interval.hpp"

// #include "doughnutPolygon.hpp"

namespace gtl = boost::polygon;
using namespace boost::polygon::operators;

int main(int argc, char const *argv[])
{
    Interval iv1(3, 7);
    Interval *iv3 = new Interval(iv1);
    

    std::cout << *iv3 << std::endl;

    std::cout << (iv1 == *iv3) << std::endl;

    std::cout << boost::polygon::contains<Interval>(iv1, 2, true);
    return 0;
}
