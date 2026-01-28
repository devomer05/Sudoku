#include "Sudoku.h"
#include <iostream>
#include <fstream>

bool Sudoku::usedInRow(uint8_t row, uint8_t val) const
{
	for (uint8_t col = 0; col < NUMBER_COUNT; col++)
		if (data[POS(row, col)] == val)
			return true;
	return false;
}

bool Sudoku::usedInColumn(uint8_t col, uint8_t val) const
{
	for (uint8_t row = 0; row < NUMBER_COUNT; row++)
		if (data[POS(row,col)] == val)
			return true;
	return false;
}

bool Sudoku::usedInBox(uint8_t x, uint8_t y, uint8_t val) const
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
}

uint8_t Sudoku::get(uint8_t x, uint8_t y) const
{
	return data[POS(x,y)];
}

void Sudoku::recomputeCandidates()
{
	for(uint8_t x = 0; x < NUMBER_COUNT; x++)
	{
		for(uint8_t y = 0; y < NUMBER_COUNT; y++)
		{
			if (get(x, y) != UNASSIGNED)
			{
				candidates[POS(x, y)] = 0;
				continue;
			}
			candidates[POS(x, y)] = FULL_MASK;
			for (uint8_t v = 1; v <= NUMBER_COUNT; v++)
			{
				if (!isSafe(x, y, v))
				{
					removeCandidate(x, y, v);
				}
			}
		}
	}
}

void Sudoku::print()
{
	std::cout << *this;
}

bool Sudoku::findUnassigned(uint8_t& x, uint8_t& y) const
{
	for (x = 0; x < NUMBER_COUNT; x++)
		for (y = 0; y < NUMBER_COUNT; y++)
			if (data[POS(x,y)] == UNASSIGNED)
				return true;
	return false;
}

bool Sudoku::findCellWithMRV(uint8_t& outRow, uint8_t& outCol) const
{
	int bestCount = 10;
	bool found = false;

	for (uint8_t r = 0; r < NUMBER_COUNT; ++r)
	{
		for (uint8_t c = 0; c < NUMBER_COUNT; ++c)
		{
			if (get(r, c) != UNASSIGNED)
				continue;

			int count = 0;
			for (uint8_t v = 1; v <= NUMBER_COUNT; ++v)
			{
				if (isSafe(r, c, v))
					count++;
			}

			// Bu dal için çelişki → erken prune
			if (count == 0)
				return false;

			if (count < bestCount)
			{
				bestCount = count;
				outRow = r;
				outCol = c;
				found = true;

				// Daha iyisi olamaz
				if (count == 1)
					return true;
			}
		}
	}

	// found == false → hiç boş hücre yok (tam dolu)
	return found;
}

uint8_t Sudoku::GetAssignedCellCount()
{
	uint8_t counter = 0;
	for (uint8_t x = 0; x < NUMBER_COUNT; x++)
		for (uint8_t y = 0; y < NUMBER_COUNT; y++)
			if (data[POS(x,y)] != UNASSIGNED)
				counter++;
	return counter;
}

bool Sudoku::loadFromFile(std::string input)
{
	std::ifstream in(input);
	if (!in.is_open())
	{
		std::cout << "Could not open Sudoku Input File: " << input << std::endl;
		return false;
	}
	try {
		in >> *this;
	} catch (const std::runtime_error& e) {
		std::cerr << "Error loading Sudoku from file: " << e.what() << std::endl;
		in.close();
		return false;
	}
	in.close();
	return true;
}

bool Sudoku::validate() const
{
	for (int r = 0; r < 9; ++r) {
		for (int c = 0; c < 9; ++c) {

			uint8_t val = data[POS(r, c)];

			if (val == UNASSIGNED)
				continue;

			// --- SATIR KONTROLÜ ---
			for (int c2 = 0; c2 < 9; ++c2) {
				if (c2 == c) continue;
				if (data[POS(r, c2)] == val) {
					std::cout << "Row check failed at (" << r << "," << c << ") with value " << (int)val << std::endl;
					return false;
				}
			}

			// --- SÜTUN KONTROLÜ ---
			for (int r2 = 0; r2 < 9; ++r2) {
				if (r2 == r) continue;
				if (data[POS(r2, c)] == val) {
					std::cout << "Column check failed at (" << r << "," << c << ") with value " << (int)val << std::endl;
					return false;
				}
			}

			// --- 3×3 KUTU KONTROLÜ ---
			int br = (r / 3) * 3;
			int bc = (c / 3) * 3;

			for (int rr = 0; rr < 3; ++rr) {
				for (int cc = 0; cc < 3; ++cc) {

					int r2 = br + rr;
					int c2 = bc + cc;

					if (r2 == r && c2 == c) continue;

					if (data[POS(r2, c2)] == val)
					{
						std::cout << "Box check failed at (" << r << "," << c << ") with value " << (int)val << std::endl;
						return false;
					}
				}
			}
		}
	}
	return true;
}

bool Sudoku::operator==(const Sudoku& other) const
{
	if (!this->isSolved() || !other.isSolved()) {
		std::cerr << "Both Sudokus must be solved to compare equality." << std::endl;
		return false;
	}
	for (int i = 0; i < NUMBER_COUNT * NUMBER_COUNT; ++i)
	{
		if (data[i] != other.data[i])
			return false;
	}
	return true;
}

bool Sudoku::operator!=(const Sudoku& other) const
{
	return !(*this == other);
}

bool Sudoku::isSolved() const
{
	for (uint8_t x = 0; x < NUMBER_COUNT; x++)
		for (uint8_t y = 0; y < NUMBER_COUNT; y++)
			if (data[POS(x, y)] == UNASSIGNED)
				return false;
	return validate();
}

std::ostream& operator<<(std::ostream& os, const Sudoku& sudoku)
{
	os << std::endl;
	os << "\t  | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |" << std::endl;
	os << "\t  -------------------------------------" << std::endl;
	for (uint8_t i = 0; i < NUMBER_COUNT; i++)
	{
		os << "\t" << i + 1 << " | ";
		for (uint8_t j = 0; j < NUMBER_COUNT; j++)
		{
			int val = (int)sudoku.data[POS(i, j)];
			val == UNASSIGNED ? os << "_" : os << val;
			os << " " << ((j + 1) % 3 == 0 ? "|" : " ") << " ";
		}
		os << std::endl;
		if ((i + 1) % 3 == 0)
			os << "\t  -------------------------------------" << std::endl;
		else
			os << "\t                                       " << std::endl;
	}
	return os;
}

std::istream& operator>>(std::istream& is, Sudoku& sudoku)
{
	memset(sudoku.data, 0, NUMBER_COUNT * NUMBER_COUNT * sizeof(uint8_t));
	for (uint8_t i = 0; i < NUMBER_COUNT; i++)
	{
		for (uint8_t j = 0; j < NUMBER_COUNT; j++)
		{
			int val;
			if (!(is >> val))
				throw std::runtime_error("Invalid sudoku input.");
			if( val < 0 || val > 9)
				throw std::runtime_error("Sudoku values must be between 0 and 9.");
			sudoku.data[POS(i, j)] = (uint8_t)val;
		}
	}
	if(!sudoku.validate())
	{ 
		throw std::runtime_error("The provided Sudoku puzzle is invalid.");
	}
	return is;
}

void Sudoku::writeRaw(std::ostream& os) const
{
	for (int i = 0; i < NUMBER_COUNT * NUMBER_COUNT; ++i)
	{
		os << (int)data[i];
		if (i + 1 < NUMBER_COUNT * NUMBER_COUNT)
			os << ' ';
	}
	os << '\n';
}

void Sudoku::readRaw(std::istream& is)
{
	for (int i = 0; i < NUMBER_COUNT * NUMBER_COUNT; ++i)
	{
		int v;
		is >> v;
		data[i] = static_cast<uint8_t>(v);
	}
}