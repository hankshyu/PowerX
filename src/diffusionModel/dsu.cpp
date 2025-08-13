//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        08/12/2025 13:48:09 
//  Module Name:        dsu.cpp
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

// Dependencies
#include "dsu.hpp"

DSU::DSU(int n){
    reset(n);
}

void DSU::reset(int n) {
    mParent.resize(n);
    mSize.assign(n, 1);
    for (int i = 0; i < n; ++i) mParent[i] = i;
    mComps = n;
}

int DSU::find(int x){
    int r = x;
    while (r != mParent[r]) r = mParent[r];
    // Path compression
    while (x != r) { int p = mParent[x]; mParent[x] = r; x = p; }
    return r;
}

// Union sets; returns true if merged, false if already in same set
bool DSU::unite(int a, int b) {
    a = find(a); b = find(b);
    if (a == b) return false;
    if (mSize[a] < mSize[b]) std::swap(a, b); // a has larger size
    mParent[b] = a;
    mSize[a] += mSize[b];
    --mComps;
    return true;
}
