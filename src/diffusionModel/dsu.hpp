//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        08/12/2025 13:48:09 
//  Module Name:        dsu.hpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        A Disjoint set union data structure 
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
//
/////////////////////////////////////////////////////////////////////////////////

#ifndef __DSU_H__
#define __DSU_H__

// Dependencies
#include <vector>

class DSU {
public:
private:
    std::vector<int> mParent;
    std::vector<int> mSize;
    int mComps = 0;

public:
    // Construct with n elements: ids 0..n-1
    explicit DSU(int n);

    void reset(int n);

    // Find representative (with path compression)
    int find(int x);

    // Union sets; returns true if merged, false if already in same set
    bool unite(int a, int b);

    // Are a and b currently in the same set?
    inline bool isConnected(int a, int b) {return find(a) == find(b);}

    // Size of the set containing x
    int getSetSize(int x) {return mSize[find(x)];}

    // Current number of components
    int getComponnetCount() const {return mComps;}
    // Current number of elements
    int getElementCount() const {return static_cast<int>(mParent.size());}


};

#endif //__DSU_H__ 