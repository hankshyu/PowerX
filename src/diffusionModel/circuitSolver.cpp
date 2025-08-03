//////////////////////////////////////////////////////////////////////////////////
//  Engineer:           Tzu-Han Hsu
//  Create Date:        07/08/2025 13:35:40
//  Module Name:        circuitSolver.cpp
//  Project Name:       PowerX
//  C++(Version):       C++17 
//  g++(Version):       Apple clang version 16.0.0 (clang-1600.0.26.6)
//  Target:             arm64-apple-darwin24.3.0
//  Thread model:       posix
//
//////////////////////////////////////////////////////////////////////////////////
//  Description:        The module of solving the resistor circuit system
//
//////////////////////////////////////////////////////////////////////////////////
//  Revision:
/////////////////////////////////////////////////////////////////////////////////

// Dependencies
// 1. C++ STL:
#include <iostream>


// 2. Boost Library:

// 3. Texo Library:
#include "circuitSolver.hpp"
// 4. 
#include "petscksp.h"

void CircuitSolver::test() {
    PetscErrorCode ierr;
    ierr = PetscInitialize(nullptr, nullptr, nullptr, nullptr); 
    CHKERRABORT(PETSC_COMM_WORLD, ierr);

    Mat G;
    Vec I, V;
    KSP ksp;
    PetscInt n = 3;

    // 1. Create sparse matrix G (3x3)
    ierr = MatCreate(PETSC_COMM_WORLD, &G); CHKERRABORT(PETSC_COMM_WORLD, ierr);
    ierr = MatSetSizes(G, PETSC_DECIDE, PETSC_DECIDE, n, n); CHKERRABORT(PETSC_COMM_WORLD, ierr);
    ierr = MatSetFromOptions(G); CHKERRABORT(PETSC_COMM_WORLD, ierr);
    ierr = MatSetUp(G); CHKERRABORT(PETSC_COMM_WORLD, ierr);

    // Fill G for 1Ω resistors in 3-node series
    ierr = MatSetValue(G, 0, 0, 1.0, INSERT_VALUES);
    ierr = MatSetValue(G, 0, 1, -1.0, INSERT_VALUES);
    ierr = MatSetValue(G, 1, 0, -1.0, INSERT_VALUES);
    ierr = MatSetValue(G, 1, 1, 2.0, INSERT_VALUES);
    ierr = MatSetValue(G, 1, 2, -1.0, INSERT_VALUES);
    ierr = MatSetValue(G, 2, 1, -1.0, INSERT_VALUES);
    ierr = MatSetValue(G, 2, 2, 1.0, INSERT_VALUES);
    ierr = MatAssemblyBegin(G, MAT_FINAL_ASSEMBLY); CHKERRABORT(PETSC_COMM_WORLD, ierr);
    ierr = MatAssemblyEnd(G, MAT_FINAL_ASSEMBLY); CHKERRABORT(PETSC_COMM_WORLD, ierr);

    // 2. Create RHS current vector I and unknown voltage vector V
    ierr = VecCreate(PETSC_COMM_WORLD, &I); CHKERRABORT(PETSC_COMM_WORLD, ierr);
    ierr = VecSetSizes(I, PETSC_DECIDE, n); CHKERRABORT(PETSC_COMM_WORLD, ierr);
    ierr = VecSetFromOptions(I); CHKERRABORT(PETSC_COMM_WORLD, ierr);
    ierr = VecDuplicate(I, &V); CHKERRABORT(PETSC_COMM_WORLD, ierr);

    // Current injection: +1A at node 1, -1A at node 2
    PetscInt idx[2] = {1, 2};
    PetscScalar val[2] = {1.0, -1.0};
    ierr = VecSetValues(I, 2, idx, val, INSERT_VALUES); CHKERRABORT(PETSC_COMM_WORLD, ierr);
    ierr = VecAssemblyBegin(I); CHKERRABORT(PETSC_COMM_WORLD, ierr);
    ierr = VecAssemblyEnd(I); CHKERRABORT(PETSC_COMM_WORLD, ierr);

    // Ground node 0
    PetscInt grounded = 0;
    ierr = MatZeroRows(G, 1, &grounded, 1.0, nullptr, I); CHKERRABORT(PETSC_COMM_WORLD, ierr);

    // 3. Solve G * V = I
    ierr = KSPCreate(PETSC_COMM_WORLD, &ksp); CHKERRABORT(PETSC_COMM_WORLD, ierr);
    ierr = KSPSetOperators(ksp, G, G); CHKERRABORT(PETSC_COMM_WORLD, ierr);
    ierr = KSPSetFromOptions(ksp); CHKERRABORT(PETSC_COMM_WORLD, ierr);
    ierr = KSPSolve(ksp, I, V); CHKERRABORT(PETSC_COMM_WORLD, ierr);

    // 4. Output voltage result
    std::cout << "[PETSc] Voltage results:\n";
    for (PetscInt i = 0; i < n; ++i) {
        PetscScalar vi;
        ierr = VecGetValues(V, 1, &i, &vi); CHKERRABORT(PETSC_COMM_WORLD, ierr);
        std::cout << "V[" << i << "] = " << vi << " V\n";
    }

    // Print current between node 1 and 2
    PetscScalar v1, v2;
    PetscInt i1 = 1, i2 = 2;
    ierr = VecGetValues(V, 1, &i1, &v1); CHKERRABORT(PETSC_COMM_WORLD, ierr);
    ierr = VecGetValues(V, 1, &i2, &v2); CHKERRABORT(PETSC_COMM_WORLD, ierr);
    PetscScalar I_12 = (v1 - v2);
    std::cout << "[PETSc] I(1→2) = " << I_12 << " A\n";

    // Cleanup
    KSPDestroy(&ksp);
    VecDestroy(&V);
    VecDestroy(&I);
    MatDestroy(&G);

    ierr = PetscFinalize(); CHKERRABORT(PETSC_COMM_WORLD, ierr);
}