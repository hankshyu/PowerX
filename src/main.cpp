#include <iostream>
#include <fstream>
#include <assert.h>
#include <unordered_map>
#include <algorithm>
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
#include "pinout.hpp"
#include "bumpMap.hpp"
#include "technology.hpp"
#include "visualiser.hpp"


// #include "doughnutPolygon.hpp"

namespace gtl = boost::polygon;
using namespace boost::polygon::operators;

int main(int argc, char const *argv[]){
    
    // CornerStitching *cs = new CornerStitching(15, 10);

    // cs->insertTile(Tile(tileType::BLOCK, Rectangle(3, 6, 10, 8)));
    // // cs->insertTile(Tile(tileType::BLOCK, Rectangle(3, 4, 8, 6)));
    // // cs->insertTile(Tile(tileType::OVERLAP, Rectangle(8, 4, 10, 6)));
    // // cs->insertTile(Tile(tileType::BLOCK, Rectangle(10, 4, 12, 6)));
    // // cs->insertTile(Tile(tileType::BLOCK, Rectangle(8, 2, 12, 4)));

    // std::string outputFile =  "./outputs/cornerStitching.txt";
    // cs->visualiseCornerStitching(outputFile);

    // Pinout rocket;
    // rocket.readFromPinoutFile("inputs/mc.txt");

    // std::ofstream of("outputs/mc.txt");
    // assert(of.is_open());
    // rocket.visualizePinOut(of);
    // of.close();

    // BumpMap bm("inputs/mc.csv");
    // std::cout << bm.getName() << std::endl;
    // bm.exportBumpMap("outputs/mc.ballmap");

    Technology tch("inputs/standard.tch");
    
    BumpMap l2b("inputs/l2.csv");
    BumpMap mcb("inputs/mc.csv");
    BumpMap nocb("inputs/noc.csv");
    BumpMap rocketb("inputs/rocket.csv");

    visualiseBumpMap(l2b, tch, "outputs/l2.bumpmap");
    visualiseBumpMap(mcb, tch, "outputs/mc.bumpmap");
    visualiseBumpMap(nocb, tch, "outputs/noc.bumpmap");
    visualiseBumpMap(rocketb, tch, "outputs/rocket.bumpmap");

    Pinout microBump("inputs/rocket64_0808.pin");
    visualisePinOut(microBump, tch, "outputs/rocket64_0808.pinout");







}