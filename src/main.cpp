#include <iostream>
#include <assert.h>
#include <unordered_map>

#include "boost/polygon/polygon.hpp"
#include "colours.hpp"
#include "units.hpp"
#include "cord.hpp"


namespace gtl = boost::polygon;
using namespace boost::polygon::operators;

template <typename Point>
void test_point() {
    //constructing a gtl point
    int x = 10;
    int y = 20;
    //Point pt(x, y);
    Point pt = gtl::construct<Point>(x, y);
    assert(gtl::x(pt) == 10);
    assert(gtl::y(pt) == 20);
   
    //a quick primer in isotropic point access
    typedef gtl::orientation_2d O;
    using gtl::HORIZONTAL;
    using gtl::VERTICAL;
    O o = HORIZONTAL;
    assert(gtl::x(pt) == gtl::get(pt, o));
   
    o = o.get_perpendicular();
    assert(o == VERTICAL);
    assert(gtl::y(pt) == gtl::get(pt, o));
   
    gtl::set(pt, o, 30);
    assert(gtl::y(pt) == 30);
   
    //using some of the library functions
    //Point pt2(10, 30);
    Point pt2 = gtl::construct<Point>(10, 30);
    assert(gtl::equivalence(pt, pt2));
   
    gtl::transformation<len_t> tr(gtl::axis_transformation::SWAP_XY);
    gtl::transform(pt, tr);
    assert(gtl::equivalence(pt, gtl::construct<Point>(30, 10)));
   
    gtl::transformation<len_t> tr2 = tr.inverse();
    assert(tr == tr2); //SWAP_XY is its own inverse transform
   
    gtl::transform(pt, tr2);
    assert(gtl::equivalence(pt, pt2)); //the two points are equal again
   
    gtl::move(pt, o, 10); //move pt 10 units in y
    assert(gtl::euclidean_distance(pt, pt2) == 10.0f);
   
    gtl::move(pt, o.get_perpendicular(), 10); //move pt 10 units in x
    assert(gtl::manhattan_distance(pt, pt2) == 20);
}

int main(int argc, char const *argv[]){

    std::cout << direction2D::DOWN << flipAngle(1.01) << " " << translateAngleToQuadrant(0.75) << std::endl;
    test_point<Cord>();
    std::cout << "Complete test!" << std::endl;

    Cord c1 (2, 7);
    Cord *c2 = new Cord(3, 7);
    Cord c3(c1);


    Cord *c4 = new Cord(*c2);
    std::cout << "Set let c4 = " << c4 << " value = " << *c4 << std::endl;


    Cord c5(*c4);
    
    c4->set(orientation2D::HORIZONTAL, -3);
    c4->set(orientation2D::VERTICAL, 12);
    
    std::cout << "c1 = " << &c1 << " value = " << c1 << std::endl;
    std::cout << "c2 = " << &c2 << " value = " << *c2 << std::endl;
    std::cout << "c3 = " << &c3 << " value = " << c3 << std::endl;
    std::cout << "c4 = " << c4 << " value = " << *c4 << std::endl;
    std::cout << "c5 = " << &c5 << " value = " << c5 << std::endl;

    std::cout << boost::polygon::equivalence(c1, *c2) << std::endl;
    std::cout << boost::polygon::equivalence(c1, c3) << std::endl;
    
    std::cout << (c1 == *c2) << " " << (c1 > *c2) << " " << (c1 >= *c2) << " " << (c1 < *c2) << " " << (c1 <= *c2) << std::endl;
    std::cout << (*c4 == c5) << " " << (*c4 > c5) << " " << (*c4 >= c5) << " " << (*c4 < c5) << " " << (*c4 <= c5) << std::endl;
    std::cout << (*c4 == *c2) << " " << (*c4 > *c2) << " " << (*c4 >= *c2) << " " << (*c4 < *c2) << " " << (*c4 <= *c2) << std::endl;

    std::cout << std::endl;

    std::swap(*c2, *c4);


    std::cout << "c1 = " << &c1 << " value = " << c1 << std::endl;
    std::cout << "c2 = " << &c2 << " value = " << *c2 << std::endl;
    std::cout << "c3 = " << &c3 << " value = " << c3 << std::endl;
    std::cout << "c4 = " << c4 << " value = " << *c4 << std::endl;
    std::cout << "c5 = " << &c5 << " value = " << c5 << std::endl;

    flen_t d1 = boost::polygon::euclidean_distance(c1, *c2);
    std::cout << d1 << std::endl;


    std::cout << "Start exp: " << std::endl;
    c1 = Cord(3, 5);
    delete c2;
    c2 = new Cord(c5);
    c3 = boost::polygon::construct<Cord>(39, 51);
    std::cout << "c1 = " << &c1 << " value = " << c1 << std::endl;
    std::cout << "c2 = " << &c2 << " value = " << *c2 << std::endl;
    std::cout << "c3 = " << &c3 << " value = " << c3 << std::endl;
    
    boost::polygon::scale_up<Cord>(*c2, 10);
    std::cout << "c2 = " << &c2 << " value = " << *c2 << std::endl;
    
    std::unordered_map<Cord, len_t> map = {{Cord(3, 5), 7}, {std::move(*c2), 12}, {c3, 100}};
    std::unordered_map<Cord, len_t>::iterator it;
    for(it = map.begin(); it != map.end(); it++){
        std::cout << it->first << " " << it->second << std::endl;
    }
    
    std::cout << "c2 = " << &c2 << " value = " << *c2 << std::endl;
    delete c2;
    std::cout << "c2 = " << &c2 << " value = " << *c2 << std::endl;

    for(it = map.begin(); it != map.end(); it++){
        std::cout << it->first << " " << it->second << std::endl;
    }


    std::cout << "Size of float: " << sizeof(float) << " bytes\n";
    std::cout << "Size of double: " << sizeof(double) << " bytes\n";
    std::cout << "Size of long double: " << sizeof(long double) << " bytes\n";

    std::cout << colours::YELLOW << "Program Exit Successfully!" << colours::COLORRST << std::endl;
    return 0;
}
