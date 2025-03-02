#include <iostream>
#include <assert.h>
#include <unordered_map>
#include "vector"

#include "boost/polygon/polygon.hpp"

#include "isotropy.hpp"
#include "rectangle.hpp"
#include "tile.hpp"
#include "rectilinear.hpp"

// #include "doughnutPolygon.hpp"

namespace gtl = boost::polygon;
using namespace boost::polygon::operators;

void print(const Tile &t){
    std::cout << t << " " << t.rt << " " << t.tr << " " << t.bl << " " << t.lb << std::endl;

}

int main(int argc, char const *argv[]){
    
    Tile *t1 = new Tile(tileType::BLOCK, Rectangle(3, 4, 7, 9));
    Tile *t2 = new Tile(tileType::BLOCK, Rectangle(5, 3, 2, 1));

    Tile *t3 = new Tile(tileType::OVERLAP, Rectangle(11, 2, 4, 7));

    Rectilinear rt(15, "blues", rectilinearType::SOFT, Rectangle(11, 15, 100, 101), 0, 0, 1, 0);
    rt.blockTiles.insert(t1);
    rt.blockTiles.insert(t2);

    rt.overlapTiles.insert(t3);

    rectilinearIllegalType rit;
    rt.isLegal(rit, 1);
    std::cout << rit << std::endl;
    
    std::vector<Cord> winding;
    rt.acquireWinding(winding, eDirection1D::COUNTERCLOCKWISE);

    for(const Cord &c : winding){
        std::cout << c << std::endl;
    }

    


}
