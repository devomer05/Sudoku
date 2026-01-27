#pragma once
#include <cstdint>
#include <cstring>
#include <string>

#define NUMBER_COUNT 9
#define UNASSIGNED 0

#define POS(x,y) (x * NUMBER_COUNT + y)

class Sudoku
{

private:
	uint8_t data[NUMBER_COUNT * NUMBER_COUNT];
	bool usedInRow(uint8_t row, uint8_t val) const;
	bool usedInColumn(uint8_t col, uint8_t val) const;
	bool usedInBox(uint8_t x, uint8_t y, uint8_t val) const;
public:

	Sudoku()
	{
		std::memset(data, 0, NUMBER_COUNT * NUMBER_COUNT * sizeof(uint8_t));
	}
	~Sudoku(){}

	void set(uint8_t x, uint8_t y, uint8_t val);
	uint8_t get(uint8_t x, uint8_t y) const;

	bool findUnassigned(uint8_t& row, uint8_t& col) const;
	bool findCellWithMRV(uint8_t& outRow, uint8_t& outCol) const;
	uint8_t GetAssignedCellCount();

	bool isSafe(uint8_t x, uint8_t y, uint8_t val) const
	{
		return !usedInRow(x, val) && !usedInBox(x, y, val) && !usedInColumn(y, val);
	}

	void print();
	bool loadFromFile(std::string input);
	void writeRaw(std::ostream& os) const;
	void readRaw(std::istream& is);
	bool validate() const;
	bool operator==(const Sudoku& other) const;
	bool operator!=(const Sudoku& other) const;
	bool isSolved() const;
	friend std::ostream& operator<<(std::ostream& os, const Sudoku& sudoku);
	friend std::istream& operator>>(std::istream& is, Sudoku& sudoku);
	
};

