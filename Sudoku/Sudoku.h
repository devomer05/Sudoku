#pragma once
#include <cstdint>
#include <cstring>
#include <string>
#include <bit>

constexpr uint16_t FULL_MASK = 0x1FF; // 9 bit: digits 1..9

#define NUMBER_COUNT 9
#define UNASSIGNED 0

#define POS(x,y) ((x) * NUMBER_COUNT + (y))

// bitwise primitives (internal)
static constexpr uint16_t bit(uint8_t v)
{
	return static_cast<uint16_t>(1u << (v - 1));
}

static inline bool singleMask(uint16_t m)
{
	return m && ((m & (m - 1)) == 0);
}

static inline uint8_t extractSingleValue(uint16_t m)
{
	return static_cast<uint8_t>(std::countr_zero(m) + 1);
}

class Sudoku
{

private:
	uint8_t data[NUMBER_COUNT * NUMBER_COUNT];
	uint16_t candidates[NUMBER_COUNT * NUMBER_COUNT];
	bool usedInRow(uint8_t row, uint8_t val) const;
	bool usedInColumn(uint8_t col, uint8_t val) const;
	bool usedInBox(uint8_t x, uint8_t y, uint8_t val) const;
	
public:

	Sudoku()
	{
		std::memset(data, 0, sizeof(data));
		std::memset(candidates, 0, sizeof(candidates));
	}
	~Sudoku(){}

	const uint8_t* rawGrid() const { return data; }
	uint8_t* rawGridMutable() { return data; }

	const uint16_t* candidatesData() const { return candidates; }
	uint16_t* rawCandidatesMutable() { return candidates; }


	void set(uint8_t x, uint8_t y, uint8_t val);
	uint8_t get(uint8_t x, uint8_t y) const;

	bool findUnassigned(uint8_t& row, uint8_t& col) const;
	bool findCellWithMRV(uint8_t& outRow, uint8_t& outCol) const;
	uint8_t GetAssignedCellCount();

	bool isSafe(uint8_t x, uint8_t y, uint8_t val) const
	{
		return !usedInRow(x, val) && !usedInBox(x, y, val) && !usedInColumn(y, val);
	}

	// --- Candidate modifiers ---  ///////////////////////////////////////////////////////////////////
	
	uint16_t getCandidates(uint8_t x, uint8_t y) const { return candidates[POS(x, y)]; }

	bool hasCandidate(uint8_t x, uint8_t y, uint8_t v) const { return (candidates[POS(x, y)] & bit(v)) != 0; }

	void addCandidate(uint8_t x, uint8_t y, uint8_t v) { candidates[POS(x, y)] |= bit(v); }

	inline bool removeCandidate(uint8_t x, uint8_t y, uint8_t v)
	{
		uint16_t before = candidates[POS(x, y)];
		candidates[POS(x, y)] &= ~bit(v);
		return before != candidates[POS(x, y)];
	}

	bool isCandidateEmpty(uint8_t x, uint8_t y) const { return candidates[POS(x, y)] == 0; }

	bool hasSingleCandidate(uint8_t x, uint8_t y) const { return singleMask(candidates[POS(x, y)]); }

	uint8_t getSingleCandidate(uint8_t x, uint8_t y) const {
		uint16_t m = candidates[POS(x, y)];
		if (!singleMask(m))
			return 0;
		return extractSingleValue(m);
	}

	// remove-only candidate propagation after set(x,y,val)
	inline void updateCandidatesAfterSet(uint8_t row, uint8_t col, uint8_t val)
	{
		uint16_t mask = ~bit(val);

		// row
		for (uint8_t c = 0; c < 9; ++c)
			if (get(row, c) == UNASSIGNED)
				candidates[POS(row, c)] &= mask;

		// column
		for (uint8_t r = 0; r < 9; ++r)
			if (get(r, col) == UNASSIGNED)
				candidates[POS(r, col)] &= mask;

		// box
		uint8_t br = (row / 3) * 3;
		uint8_t bc = (col / 3) * 3;
		for (uint8_t dr = 0; dr < 3; ++dr)
			for (uint8_t dc = 0; dc < 3; ++dc)
			{
				uint8_t r = br + dr;
				uint8_t c = bc + dc;
				if (get(r, c) == UNASSIGNED)
					candidates[POS(r, c)] &= mask;
			}
	}

	inline bool removeCandidatesMask(uint8_t r, uint8_t c, uint16_t killMask)
	{
		uint16_t before = candidates[POS(r, c)];
		candidates[POS(r, c)] &= ~killMask;
		return before != candidates[POS(r, c)];
	}

	void recomputeCandidates();
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

