//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        07/017/2025 18:54:20
//  Module Name:        router.cpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        For testing gurobi and run flow based routing algorithm
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
// 07/08/2025           Ported from diffusionSimulator class
/////////////////////////////////////////////////////////////////////////////////

#include "router.hpp"

void testRoute(){
    try{
        GRBEnv env = GRBEnv(true);
        env.set(GRB_IntParam_OutputFlag, 1);
        env.start();
        GRBModel model = GRBModel(env);

        // Step 1: Build valid GNodes
        
        std::vector<std::vector<std::vector<GNode>>> metalNodes;
        std::vector<std::vector<std::vector<GNode>>> viaNodes;

        // create nodes
        metalNodes.resize(3);
        for (int z = 0; z < LAYERS; ++z) {
            metalNodes[z].resize(HEIGHT);
            for (int y = 0; y < HEIGHT; ++y) {
                metalNodes[z][y].resize(WIDTH);
                for (int x = 0; x < WIDTH; ++x) {
                    metalNodes[z][y][x] = GNode(true, z, y, x);
                }
            }
        }

        int VIA_LAYERS = LAYERS - 1;
        int VIA_HEIGHT = HEIGHT/2;
        int VIA_WIDTH = WIDTH/2;

        viaNodes.resize(VIA_LAYERS);
        for(int z = 0; z < VIA_LAYERS; ++z){
            viaNodes[z].resize(VIA_HEIGHT);
            for(int y = 0; y < VIA_HEIGHT; ++y){
                viaNodes[z][y].resize(VIA_WIDTH);
                for(int x = 0; x < VIA_WIDTH; ++x){
                    viaNodes[z][y][x] = GNode(false, z, y, x);
                }
            }
        }

        std::vector<std::vector<std::vector<std::vector<GEdge>>>> metalEdges;
        metalEdges.resize(LAYERS);



        // create edges for same metal layer
        for (int z = 0; z < LAYERS; ++z) {
            metalEdges[z].resize(HEIGHT);
            for (int y = 0; y < HEIGHT; ++y) {
                metalEdges[z][y].resize(WIDTH);
                for (int x = 0; x < WIDTH; ++x) {
                    GNode *currNode = &metalNodes[z][y][x];
                    if(x != 0){
                        metalEdges[z][y][x].emplace_back(currNode, &metalNodes[z][y][x-1]);
                    }
                    if(x != (WIDTH - 1)){
                        metalEdges[z][y][x].emplace_back(currNode, &metalNodes[z][y][x+1]);
                    }
                    if(y != 0){
                        metalEdges[z][y][x].emplace_back(currNode, &metalNodes[z][y-1][x]);
                    }
                    if(y != (HEIGHT-1)){
                        metalEdges[z][y][x].emplace_back(currNode, &metalNodes[z][y+1][x]);
                    }
                }
            }
        }


        std::vector<std::vector<std::vector<std::vector<GEdge>>>> viaEdges;
        viaEdges.resize(VIA_LAYERS);
        // create edges for vias
        
        for(int z = 0; z < VIA_LAYERS; ++z){
            viaEdges[z].resize(VIA_HEIGHT);
            for(int y = 0; y < VIA_HEIGHT; ++y){
                viaEdges[z][y].resize(VIA_WIDTH);
                for(int x = 0; x < VIA_WIDTH; ++x){
                    GNode *currVia = &viaNodes[z][y][x];
                    
                    GNode *upLLMetal = &viaNodes[z][2*y+0][2*x+0];
                    GNode *upLRMetal = &viaNodes[z][2*y+0][2*x+1];
                    GNode *upULMetal = &viaNodes[z][2*y+1][2*x+0];
                    GNode *upURMetal = &viaNodes[z][2*y+1][2*x+1];

                    GNode *downLLMetal = &viaNodes[z+1][2*y+0][2*x+0];
                    GNode *downLRMetal = &viaNodes[z+1][2*y+0][2*x+1];
                    GNode *downULMetal = &viaNodes[z+1][2*y+1][2*x+0];
                    GNode *downURMetal = &viaNodes[z+1][2*y+1][2*x+1];

                    viaEdges[z][y][x].emplace_back(currVia, upLLMetal);
                    viaEdges[z][y][x].emplace_back(currVia, upLRMetal);
                    viaEdges[z][y][x].emplace_back(currVia, upULMetal);
                    viaEdges[z][y][x].emplace_back(currVia, upURMetal);
                    

                    viaEdges[z][y][x].emplace_back(downLLMetal, currVia);
                    viaEdges[z][y][x].emplace_back(downLRMetal, currVia);
                    viaEdges[z][y][x].emplace_back(downULMetal, currVia);
                    viaEdges[z][y][x].emplace_back(downURMetal, currVia);
                }
            }
        }

        // create variables according to the value




    } catch (GRBException& e) {
        std::cerr << "Gurobi error: " << e.getMessage() << std::endl;
    } catch (...) {
        std::cerr << "Unexpected error occurred." << std::endl;
    }
}