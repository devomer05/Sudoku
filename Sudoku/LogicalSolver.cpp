#include "LogicalSolver.h"
#include "BacktrackingSolverMRV.h"

SolveResult LogicalSolver::solve(Sudoku& sudoku)
{
    if (sudoku.isSolved())
		return SolveResult::AlreadySolved;

	sudoku.recomputeCandidates();

	while (!sudoku.isSolved() && applyLogicalStep(sudoku));
	
	if(sudoku.isSolved()) return SolveResult::SolvedByLogical;

	return BacktrackingSolverMRV().solve(sudoku);
}

bool LogicalSolver::applyLogicalStep(Sudoku& s)
{
    // set yapanlar
    if (applyNakedSingle(s))  return true;
    if (applyHiddenSingle(s)) return true;

    // sadece candidate silenler
    if (applyLockedCandidatesPointing(s))  return true;
    if (applyLockedCandidatesClaiming(s))  return true;
    if (applyNakedPair(s))                 return true;
    if (applyHiddenPair(s))                return true;

    return false;
}

bool LogicalSolver::applyNakedSingle(Sudoku& sudoku)
{
	bool progressed = false;

	for (uint8_t r = 0; r < 9; ++r)
	{
		for (uint8_t c = 0; c < 9; ++c)
		{
			if (sudoku.get(r, c) != UNASSIGNED)
				continue;
			if (!sudoku.hasSingleCandidate(r, c))
				continue;
			uint8_t value = sudoku.getSingleCandidate(r, c);
			sudoku.set(r, c, value);
			sudoku.updateCandidatesAfterSet(r, c, value);
			progressed = true;
		}
	}
	return progressed;
}

bool LogicalSolver::applyHiddenSingle(Sudoku& sudoku)
{
    bool progressed = false;

    // Rows
    for (uint8_t row = 0; row < 9; ++row)
    {
        for (uint8_t digit = 1; digit <= 9; ++digit)
        {
            int candidateCount = 0;
            uint8_t targetCol = 0;

            for (uint8_t col = 0; col < 9 && candidateCount <= 1; ++col)
            {
                if (sudoku.get(row, col) == UNASSIGNED &&
                    sudoku.hasCandidate(row, col, digit))
                {
                    targetCol = col;
                    ++candidateCount;
                }
            }

            if (candidateCount == 1)
            {
                sudoku.set(row, targetCol, digit);
				sudoku.updateCandidatesAfterSet(row, targetCol, digit);
                progressed = true;
            }
        }
    }

    // Columns
    for (uint8_t col = 0; col < 9; ++col)
    {
        for (uint8_t digit = 1; digit <= 9; ++digit)
        {
            int candidateCount = 0;
            uint8_t targetRow = 0;

            for (uint8_t row = 0; row < 9 && candidateCount <= 1; ++row)
            {
                if (sudoku.get(row, col) == UNASSIGNED &&
                    sudoku.hasCandidate(row, col, digit))
                {
                    targetRow = row;
                    ++candidateCount;
                }
            }

            if (candidateCount == 1)
            {
                sudoku.set(targetRow, col, digit);
				sudoku.updateCandidatesAfterSet(targetRow, col, digit);
                progressed = true;
            }
        }
    }

    // Boxes
    for (uint8_t boxRow = 0; boxRow < 9; boxRow += 3)
    {
        for (uint8_t boxCol = 0; boxCol < 9; boxCol += 3)
        {
            for (uint8_t digit = 1; digit <= 9; ++digit)
            {
                int candidateCount = 0;
                uint8_t targetRow = 0, targetCol = 0;

                for (uint8_t dr = 0; dr < 3 && candidateCount <= 1; ++dr)
                {
                    for (uint8_t dc = 0; dc < 3 && candidateCount <= 1; ++dc)
                    {
                        uint8_t row = boxRow + dr;
                        uint8_t col = boxCol + dc;

                        if (sudoku.get(row, col) == UNASSIGNED &&
                            sudoku.hasCandidate(row, col, digit))
                        {
                            targetRow = row;
                            targetCol = col;
                            ++candidateCount;
                        }
                    }
                }

                if (candidateCount == 1)
                {
                    sudoku.set(targetRow, targetCol, digit);
					sudoku.updateCandidatesAfterSet(targetRow, targetCol, digit);
                    progressed = true;
                }
            }
        }
    }
    return progressed;
}

bool LogicalSolver::applyLockedCandidatesPointing(Sudoku& sudoku)
{
    bool progressed = false;

    for (uint8_t boxRow = 0; boxRow < 9; boxRow += 3)
    {
        for (uint8_t boxCol = 0; boxCol < 9; boxCol += 3)
        {
            for (uint8_t digit = 1; digit <= 9; ++digit)
            {
                int count = 0;
                uint8_t lockedRow = 0;
                uint8_t lockedCol = 0;

                // Kutu içini tara
                for (uint8_t dr = 0; dr < 3; ++dr)
                {
                    for (uint8_t dc = 0; dc < 3; ++dc)
                    {
                        uint8_t row = boxRow + dr;
                        uint8_t col = boxCol + dc;

                        if (sudoku.get(row, col) == UNASSIGNED &&
                            sudoku.hasCandidate(row, col, digit))
                        {
                            if (count == 0)
                            {
                                lockedRow = row;
                                lockedCol = col;
                            }
                            else
                            {
                                if (row != lockedRow) lockedRow = 255;
                                if (col != lockedCol) lockedCol = 255;
                            }
                            ++count;
                        }
                    }
                }

                if (count <= 1)
                    continue;

                // Pointing Row
                if (lockedRow != 255)
                {
                    for (uint8_t col = 0; col < 9; ++col)
                    {
                        if (col >= boxCol && col < boxCol + 3)
                            continue;

                        if (sudoku.hasCandidate(lockedRow, col, digit))
                        {
                            sudoku.removeCandidate(lockedRow, col, digit);
                            progressed = true;
                        }
                    }
                }

                // Pointing Column
                if (lockedCol != 255)
                {
                    for (uint8_t row = 0; row < 9; ++row)
                    {
                        if (row >= boxRow && row < boxRow + 3)
                            continue;

                        if (sudoku.hasCandidate(row, lockedCol, digit))
                        {
                            sudoku.removeCandidate(row, lockedCol, digit);
                            progressed = true;
                        }
                    }
                }
            }
        }
    }

    return progressed;
}

bool LogicalSolver::applyLockedCandidatesClaiming(Sudoku& sudoku)
{
    bool progressed = false;

    // Rows -> Box
    for (uint8_t row = 0; row < 9; ++row)
    {
        for (uint8_t digit = 1; digit <= 9; ++digit)
        {
            int count = 0;
            uint8_t lockedBoxRow = 0;
            uint8_t lockedBoxCol = 0;

            for (uint8_t col = 0; col < 9; ++col)
            {
                if (sudoku.get(row, col) == UNASSIGNED &&
                    sudoku.hasCandidate(row, col, digit))
                {
                    uint8_t boxRow = (row / 3) * 3;
                    uint8_t boxCol = (col / 3) * 3;

                    if (count == 0)
                    {
                        lockedBoxRow = boxRow;
                        lockedBoxCol = boxCol;
                    }
                    else
                    {
                        if (boxRow != lockedBoxRow ||
                            boxCol != lockedBoxCol)
                        {
                            lockedBoxRow = 255;
                        }
                    }
                    ++count;
                }
            }

            if (count <= 1 || lockedBoxRow == 255)
                continue;

            for (uint8_t dr = 0; dr < 3; ++dr)
            {
                for (uint8_t dc = 0; dc < 3; ++dc)
                {
                    uint8_t r = lockedBoxRow + dr;
                    uint8_t c = lockedBoxCol + dc;

                    if (r == row)
                        continue;

                    if (sudoku.hasCandidate(r, c, digit))
                    {
                        sudoku.removeCandidate(r, c, digit);
                        progressed = true;
                    }
                }
            }
        }
    }

    // Columns -> Box
    for (uint8_t col = 0; col < 9; ++col)
    {
        for (uint8_t digit = 1; digit <= 9; ++digit)
        {
            int count = 0;
            uint8_t lockedBoxRow = 0;
            uint8_t lockedBoxCol = 0;

            for (uint8_t row = 0; row < 9; ++row)
            {
                if (sudoku.get(row, col) == UNASSIGNED &&
                    sudoku.hasCandidate(row, col, digit))
                {
                    uint8_t boxRow = (row / 3) * 3;
                    uint8_t boxCol = (col / 3) * 3;

                    if (count == 0)
                    {
                        lockedBoxRow = boxRow;
                        lockedBoxCol = boxCol;
                    }
                    else
                    {
                        if (boxRow != lockedBoxRow ||
                            boxCol != lockedBoxCol)
                        {
                            lockedBoxRow = 255;
                        }
                    }
                    ++count;
                }
            }

            if (count <= 1 || lockedBoxRow == 255)
                continue;

            for (uint8_t dr = 0; dr < 3; ++dr)
            {
                for (uint8_t dc = 0; dc < 3; ++dc)
                {
                    uint8_t r = lockedBoxRow + dr;
                    uint8_t c = lockedBoxCol + dc;

                    if (c == col)
                        continue;

                    if (sudoku.hasCandidate(r, c, digit))
                    {
                        sudoku.removeCandidate(r, c, digit);
                        progressed = true;
                    }
                }
            }
        }
    }

    return progressed;
}

bool LogicalSolver::applyNakedPair(Sudoku& s)
{
    bool changed = false;

    auto isPair = [](uint16_t m) {
        return std::popcount(m) == 2;
        };

    // ========= ROW =========
    for (uint8_t r = 0; r < 9; ++r)
        for (uint8_t c1 = 0; c1 < 8; ++c1)
        {
            if (s.get(r, c1) != UNASSIGNED) continue;

            uint16_t m = s.getCandidates(r, c1);
            if (!isPair(m)) continue;

            for (uint8_t c2 = c1 + 1; c2 < 9; ++c2)
            {
                if (s.get(r, c2) != UNASSIGNED) continue;
                if (s.getCandidates(r, c2) != m) continue;

                uint16_t kill = m;
                for (uint8_t c = 0; c < 9; ++c)
                {
                    if (c == c1 || c == c2) continue;
                    if (s.get(r, c) != UNASSIGNED) continue;

                    changed |= s.removeCandidatesMask(r, c, kill);
                }
            }
        }

    // ========= COLUMN =========
    for (uint8_t c = 0; c < 9; ++c)
        for (uint8_t r1 = 0; r1 < 8; ++r1)
        {
            if (s.get(r1, c) != UNASSIGNED) continue;

            uint16_t m = s.getCandidates(r1, c);
            if (!isPair(m)) continue;

            for (uint8_t r2 = r1 + 1; r2 < 9; ++r2)
            {
                if (s.get(r2, c) != UNASSIGNED) continue;
                if (s.getCandidates(r2, c) != m) continue;

                uint16_t kill = m;
                for (uint8_t r = 0; r < 9; ++r)
                {
                    if (r == r1 || r == r2) continue;
                    if (s.get(r, c) != UNASSIGNED) continue;

                    changed |= s.removeCandidatesMask(r, c, kill);
                }
            }
        }

    // ========= BOX =========
    for (uint8_t br = 0; br < 9; br += 3)
        for (uint8_t bc = 0; bc < 9; bc += 3)
            for (uint8_t i = 0; i < 8; ++i)
            {
                uint8_t r1 = br + i / 3;
                uint8_t c1 = bc + i % 3;
                if (s.get(r1, c1) != UNASSIGNED) continue;

                uint16_t m = s.getCandidates(r1, c1);
                if (!isPair(m)) continue;

                for (uint8_t j = i + 1; j < 9; ++j)
                {
                    uint8_t r2 = br + j / 3;
                    uint8_t c2 = bc + j % 3;
                    if (s.get(r2, c2) != UNASSIGNED) continue;
                    if (s.getCandidates(r2, c2) != m) continue;

                    uint16_t kill = m;
                    for (uint8_t k = 0; k < 9; ++k)
                    {
                        uint8_t r = br + k / 3;
                        uint8_t c = bc + k % 3;
                        if ((r == r1 && c == c1) || (r == r2 && c == c2)) continue;
                        if (s.get(r, c) != UNASSIGNED) continue;

                        changed |= s.removeCandidatesMask(r, c, kill);
                    }
                }
            }

    return changed;
}

bool LogicalSolver::applyHiddenPair(Sudoku& s)
{
    bool changed = false;

    // ========== ROW ==========
    for (uint8_t r = 0; r < 9; ++r)
    {
        uint16_t pos[10] = {}; // digit -> 9-bit cell mask

        // her digit hangi kolonlarda olabilir
        for (uint8_t c = 0; c < 9; ++c)
        {
            if (s.get(r, c) != UNASSIGNED) continue;
            uint16_t m = s.getCandidates(r, c);
            for (uint8_t d = 1; d <= 9; ++d)
                if (m & bit(d))
                    pos[d] |= (1u << c);
        }

        for (uint8_t a = 1; a <= 8; ++a)
            for (uint8_t b = a + 1; b <= 9; ++b)
            {
                if (pos[a] == pos[b] && std::popcount(pos[a]) == 2)
                {
                    uint16_t keep = bit(a) | bit(b);
                    for (uint8_t c = 0; c < 9; ++c)
                        if (pos[a] & (1u << c))
                            changed |= s.removeCandidatesMask(r, c, ~keep);
                }
            }
    }

    // ========== COLUMN ==========
    for (uint8_t c = 0; c < 9; ++c)
    {
        uint16_t pos[10] = {};

        for (uint8_t r = 0; r < 9; ++r)
        {
            if (s.get(r, c) != UNASSIGNED) continue;
            uint16_t m = s.getCandidates(r, c);
            for (uint8_t d = 1; d <= 9; ++d)
                if (m & bit(d))
                    pos[d] |= (1u << r);
        }

        for (uint8_t a = 1; a <= 8; ++a)
            for (uint8_t b = a + 1; b <= 9; ++b)
            {
                if (pos[a] == pos[b] && std::popcount(pos[a]) == 2)
                {
                    uint16_t keep = bit(a) | bit(b);
                    for (uint8_t r = 0; r < 9; ++r)
                        if (pos[a] & (1u << r))
                            changed |= s.removeCandidatesMask(r, c, ~keep);
                }
            }
    }

    // ========== BOX ==========
    for (uint8_t br = 0; br < 9; br += 3)
        for (uint8_t bc = 0; bc < 9; bc += 3)
        {
            uint16_t pos[10] = {};

            for (uint8_t i = 0; i < 9; ++i)
            {
                uint8_t r = br + i / 3;
                uint8_t c = bc + i % 3;
                if (s.get(r, c) != UNASSIGNED) continue;

                uint16_t m = s.getCandidates(r, c);
                for (uint8_t d = 1; d <= 9; ++d)
                    if (m & bit(d))
                        pos[d] |= (1u << i);
            }

            for (uint8_t a = 1; a <= 8; ++a)
                for (uint8_t b = a + 1; b <= 9; ++b)
                {
                    if (pos[a] == pos[b] && std::popcount(pos[a]) == 2)
                    {
                        uint16_t keep = bit(a) | bit(b);
                        for (uint8_t i = 0; i < 9; ++i)
                            if (pos[a] & (1u << i))
                            {
                                uint8_t r = br + i / 3;
                                uint8_t c = bc + i % 3;
                                changed |= s.removeCandidatesMask(r, c, ~keep);
                            }
                    }
                }
        }

    return changed;
}