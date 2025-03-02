#include <iostream>
#include <assert.h>
#include <unordered_map>
#include "vector"

#include "boost/polygon/polygon.hpp"

// #include "isotropy.hpp"
// #include "rectangle.hpp"
// #include "tile.hpp"
// #include "rectilinear.hpp"

#include "line.hpp"
#include "tile.hpp"
#include "lineTile.hpp"
#include "cornerStitching.hpp"

// #include "doughnutPolygon.hpp"

namespace gtl = boost::polygon;
using namespace boost::polygon::operators;

int main(int argc, char const *argv[]){
    
    CornerStitching cs(15, 10);
    cs.insertTile(Tile(tileType::BLOCK, Rectangle(3, 2, 6, 7)));

    std::unordered_set<Tile *> alltiles;
    cs.collectAllTiles(alltiles);

    for(const Tile *t : alltiles){
        std::cout << *t << std::endl;
    }
    


}