#include <iostream>
#include <assert.h>
#include <unordered_map>
#include "vector"

#include "boost/polygon/polygon.hpp"

#include "isotropy.hpp"
#include "cord.hpp"
#include "fcord.hpp"

// #include "doughnutPolygon.hpp"

namespace gtl = boost::polygon;
using namespace boost::polygon::operators;

// int main(int argc, char const *argv[]){
//     Rectangle r1(3, 9, 11, 17);
//     Tile t1(7, 9, 11, 14);

//     std::cout << boost::polygon::delta<Rectangle>(boost::polygon::rectangle_data<len_t>(3, 9, 11, 17), eOrientation2D::HORIZONTAL) << std::endl;

// }
#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    Cord c1 = Cord(3, 5);
    FCord c2(3.5, 7.5);
    std::cout << c1 << std::endl;
    std::cout << c2 << std::endl;
    FCord c3(c1);
    std::cout << (c1 >= c2) << std::endl;
    std::cout << c3 << std::endl;
}