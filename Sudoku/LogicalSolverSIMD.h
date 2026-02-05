#pragma once
#include "LogicalSolver.h"

class LogicalSolverSIMD : public  LogicalSolver
{
protected:
	bool applyNakedSingle(Sudoku& s) override; // Add this line to declare the override
public:
	const char* getName() const override { return "Logical Solver SIMD"; }

};
