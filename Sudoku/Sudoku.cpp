#include "Sudoku.h"
#include <iostream>
#include <fstream>

bool Sudoku::usedInRow(uint8_t row, uint8_t val)
{
	for (uint8_t col = 0; col < NUMBER_COUNT; col++)
		if (data[POS(row, col)] == val)
			return true;
	return false;
}

bool Sudoku::usedInColumn(uint8_t col, uint8_t val)
{
	for (uint8_t row = 0; row < NUMBER_COUNT; row++)
		if (data[POS(row,col)] == val)
			return true;
	return false;
}

bool Sudoku::usedInBox(uint8_t x, uint8_t y, uint8_t val)
{
	uint8_t boxStartRow = x - x % 3;
	uint8_t boxStartCol = y - y % 3;
	for (uint8_t row = 0; row < NUMBER_COUNT / 3; row++)
		for (uint8_t col = 0; col < NUMBER_COUNT / 3; col++)
			if (data[POS ( (boxStartRow + row ), (boxStartCol + col) )] == val)
				return true;
	return false;
}

void Sudoku::set(uint8_t x, uint8_t y, uint8_t val)
{
	data[POS(x,y)] = val;
	lastX = x;
	lastY = y;
}

uint8_t Sudoku::get(uint8_t x, uint8_t y)
{
	return data[POS(x,y)];
}

void Sudoku::print()
{
	std::cout << std::endl;
	/* 
	if (lastX >= 0)
	{
		std::cout << "\tLast Cell: ( " << lastX + 1 << "," << lastY + 1 << " ) =  " << data[lastX,lastY] << std::endl;
		std::cout << "\tExplanation: " << lastMessage << std::endl << std::endl;
	}
	*/

	std::cout << "\t  | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |" << std::endl;
	std::cout << "\t  -------------------------------------" << std::endl;
	for (uint8_t i = 0; i < NUMBER_COUNT; i++)
	{
		std::cout << "\t" << i + 1 << " | ";
		for (uint8_t j = 0; j < NUMBER_COUNT; j++)
		{
			bool isLastSetBox = false;
			
			isLastSetBox = (i == lastX && j == lastY);
			
			std::cout << data[POS(i,j)] << (isLastSetBox ? "*" : " ") << ((j + 1) % 3 == 0 ? "|" : " ") << " ";
		}
		std::cout << std::endl;
		if ((i + 1) % 3 == 0)
			std::cout << "\t-------------------------------------" << std::endl;
		else
			std::cout << "\t                                     " << std::endl;
	}
	lastX = lastY = -1;
}

bool Sudoku::getFirstUnassignedPos(uint8_t& x, uint8_t& y)
{
	for (x = 0; x < NUMBER_COUNT; x++)
		for (y = 0; y < NUMBER_COUNT; y++)
			if (data[POS(x,y)] == UNASSIGNED)
				return true;
	return false;
}

uint8_t Sudoku::GetAssignedCellCount()
{
	int counter = 0;
	for (uint8_t x = 0; x < NUMBER_COUNT; x++)
		for (uint8_t y = 0; y < NUMBER_COUNT; y++)
			if (data[POS(x,y)] != UNASSIGNED)
				counter++;
	return counter;
}

bool Sudoku::init(std::string input)
{
	lastX = lastY = -1;
	memset(data, 0, NUMBER_COUNT * NUMBER_COUNT * sizeof(uint8_t));

	std::ifstream in(input);
	if (!in.good())
	{
		return false;
	}
	for (uint8_t i = 0; i < NUMBER_COUNT; i++)
	{
		for (uint8_t j = 0; j < NUMBER_COUNT; j++)
		{
			in >> data[POS(i,j)];
		}
	}
	in.close();
	return true;
}
