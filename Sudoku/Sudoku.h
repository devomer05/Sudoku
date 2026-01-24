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
	bool usedInRow(uint8_t row, uint8_t val);
	bool usedInColumn(uint8_t col, uint8_t val);
	bool usedInBox(uint8_t x, uint8_t y, uint8_t val);
public:

	Sudoku()
	{
		std::memset(data, 0, NUMBER_COUNT * NUMBER_COUNT * sizeof(uint8_t));
	}
	~Sudoku(){}

	void set(uint8_t x, uint8_t y, uint8_t val);
	uint8_t get(uint8_t x, uint8_t y);

	bool findUnassigned(uint8_t& row, uint8_t& col);
	uint8_t GetAssignedCellCount();

	bool isSafe(uint8_t x, uint8_t y, uint8_t val)
	{
		return !usedInRow(x, val) && !usedInBox(x, y, val) && !usedInColumn(y, val);
	}

	void print();
	bool loadFromFile(std::string input);
	bool validate() const;
	bool operator==(const Sudoku& other) const;
	bool isSolved() const;
	friend std::ostream& operator<<(std::ostream& os, const Sudoku& sudoku);
	friend std::istream& operator>>(std::istream& is, Sudoku& sudoku);
	
};

