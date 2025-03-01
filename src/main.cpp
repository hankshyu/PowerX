#include <iostream>
#include <assert.h>
#include <unordered_map>
#include "vector"

#include "boost/polygon/polygon.hpp"

#include "rectangle.hpp"
#include "tile.hpp"

// #include "doughnutPolygon.hpp"

namespace gtl = boost::polygon;
using namespace boost::polygon::operators;

void print(const Tile &t){
    std::cout << t << " " << t.rt << " " << t.tr << " " << t.bl << " " << t.lb << std::endl;

}

int main(int argc, char const *argv[]){



    Tile *t1 = new Tile(tileType::BLANK, Rectangle(1, 11, 111, 1111));
    Tile *t2 = new Tile(tileType::BLOCK, Rectangle(2, 22, 222, 2222));
    Tile *t3 = new Tile(tileType::OVERLAP, Rectangle(3, 33, 333, 3333));
    Tile *t4 = new Tile(tileType::BLOCK, Rectangle(4, 44, 444, 4444));
    Tile *t5 = new Tile(tileType::BLANK, Rectangle(5, 55, 555, 5555));


    
    t1->rt = t2;
    t1->tr = t3;
    t1->bl = t4;
    t1->lb = t5;

    Tile *t6 = new Tile(*t1);
    Tile t7(*t1);
    Tile t8 = *t1;


    print(*t1);
    print(*t6);
    print(t7);
    print(t8);

    




}
