#include "LogicalSolver.h"
#include "BacktrackingSolverMRV.h"

SolveResult LogicalSolver::solve(Sudoku& sudoku)
{
    if (sudoku.isSolved())
        return SolveResult::AlreadySolved;

    sudoku.recomputeCandidates();

    while (!sudoku.isSolved() && applyLogicalStep(sudoku));

    if (sudoku.isSolved()) return SolveResult::SolvedByLogical;

    return BacktrackingSolverMRV().solve(sudoku);
}

bool LogicalSolver::applyLogicalStep(Sudoku& s)
{
    if (applyNakedSingle(s))  return true;
    if (applyHiddenSingle(s)) return true;

    if (applyLockedCandidatesPointing(s))  return true;
    if (applyLockedCandidatesClaiming(s))  return true;
    if (applyNakedPair(s))                 return true;
    if (applyHiddenPair(s))                return true;

    return false;
}

bool LogicalSolver::applyNakedSingle(Sudoku& sudoku)
{
    bool progressed = false;
    uint32_t localSet = 0;

    for (uint8_t r = 0; r < 9; ++r)
        for (uint8_t c = 0; c < 9; ++c)
        {
            if (sudoku.get(r, c) != UNASSIGNED) continue;
            if (!sudoku.hasSingleCandidate(r, c)) continue;

            uint8_t value = sudoku.getSingleCandidate(r, c);
            sudoku.set(r, c, value);
            sudoku.updateCandidatesAfterSet(r, c, value);

            progressed = true;
            ++localSet;
        }

    if (progressed) {
        logicalStats.data[LS_NAKED_SINGLE][0]++;
        logicalStats.data[LS_NAKED_SINGLE][1] += localSet;
    }
    return progressed;
}

bool LogicalSolver::applyHiddenSingle(Sudoku& sudoku)
{
    bool progressed = false;
    uint32_t localSet = 0;

    // Rows
    for (uint8_t row = 0; row < 9; ++row)
        for (uint8_t digit = 1; digit <= 9; ++digit)
        {
            int candidateCount = 0;
            uint8_t targetCol = 0;

            for (uint8_t col = 0; col < 9 && candidateCount <= 1; ++col)
                if (sudoku.get(row, col) == UNASSIGNED &&
                    sudoku.hasCandidate(row, col, digit))
                {
                    targetCol = col;
                    ++candidateCount;
                }

            if (candidateCount == 1) {
                sudoku.set(row, targetCol, digit);
                sudoku.updateCandidatesAfterSet(row, targetCol, digit);
                progressed = true;
                ++localSet;
            }
        }

    // Columns
    for (uint8_t col = 0; col < 9; ++col)
        for (uint8_t digit = 1; digit <= 9; ++digit)
        {
            int candidateCount = 0;
            uint8_t targetRow = 0;

            for (uint8_t row = 0; row < 9 && candidateCount <= 1; ++row)
                if (sudoku.get(row, col) == UNASSIGNED &&
                    sudoku.hasCandidate(row, col, digit))
                {
                    targetRow = row;
                    ++candidateCount;
                }

            if (candidateCount == 1) {
                sudoku.set(targetRow, col, digit);
                sudoku.updateCandidatesAfterSet(targetRow, col, digit);
                progressed = true;
                ++localSet;
            }
        }

    // Boxes
    for (uint8_t boxRow = 0; boxRow < 9; boxRow += 3)
        for (uint8_t boxCol = 0; boxCol < 9; boxCol += 3)
            for (uint8_t digit = 1; digit <= 9; ++digit)
            {
                int candidateCount = 0;
                uint8_t tr = 0, tc = 0;

                for (uint8_t dr = 0; dr < 3 && candidateCount <= 1; ++dr)
                    for (uint8_t dc = 0; dc < 3 && candidateCount <= 1; ++dc)
                    {
                        uint8_t r = boxRow + dr;
                        uint8_t c = boxCol + dc;
                        if (sudoku.get(r, c) == UNASSIGNED &&
                            sudoku.hasCandidate(r, c, digit))
                        {
                            tr = r; tc = c;
                            ++candidateCount;
                        }
                    }

                if (candidateCount == 1) {
                    sudoku.set(tr, tc, digit);
                    sudoku.updateCandidatesAfterSet(tr, tc, digit);
                    progressed = true;
                    ++localSet;
                }
            }

    if (progressed) {
        logicalStats.data[LS_HIDDEN_SINGLE][0]++;
        logicalStats.data[LS_HIDDEN_SINGLE][1] += localSet;
    }
    return progressed;
}

bool LogicalSolver::applyLockedCandidatesPointing(Sudoku& sudoku)
{
    bool progressed = false;
    uint32_t localRemove = 0;

    for (uint8_t boxRow = 0; boxRow < 9; boxRow += 3)
        for (uint8_t boxCol = 0; boxCol < 9; boxCol += 3)
            for (uint8_t digit = 1; digit <= 9; ++digit)
            {
                int count = 0;
                uint8_t lockedRow = 0, lockedCol = 0;

                for (uint8_t dr = 0; dr < 3; ++dr)
                    for (uint8_t dc = 0; dc < 3; ++dc)
                    {
                        uint8_t r = boxRow + dr;
                        uint8_t c = boxCol + dc;
                        if (sudoku.get(r, c) == UNASSIGNED &&
                            sudoku.hasCandidate(r, c, digit))
                        {
                            if (count == 0) { lockedRow = r; lockedCol = c; }
                            else {
                                if (r != lockedRow) lockedRow = 255;
                                if (c != lockedCol) lockedCol = 255;
                            }
                            ++count;
                        }
                    }

                if (count <= 1) continue;

                if (lockedRow != 255)
                    for (uint8_t c = 0; c < 9; ++c)
                        if ((c < boxCol || c >= boxCol + 3) &&
                            sudoku.removeCandidate(lockedRow, c, digit))
                        {
                            progressed = true; ++localRemove;
                        }

                if (lockedCol != 255)
                    for (uint8_t r = 0; r < 9; ++r)
                        if ((r < boxRow || r >= boxRow + 3) &&
                            sudoku.removeCandidate(r, lockedCol, digit))
                        {
                            progressed = true; ++localRemove;
                        }
            }

    if (progressed) {
        logicalStats.data[LS_LOCKED_POINTING][0]++;
        logicalStats.data[LS_LOCKED_POINTING][1] += localRemove;
    }
    return progressed;
}

bool LogicalSolver::applyLockedCandidatesClaiming(Sudoku& sudoku)
{
    bool progressed = false;
    uint32_t localRemove = 0;

    for (uint8_t row = 0; row < 9; ++row)
        for (uint8_t digit = 1; digit <= 9; ++digit)
        {
            int count = 0;
            uint8_t lockedBoxRow = 0, lockedBoxCol = 0;

            for (uint8_t col = 0; col < 9; ++col)
                if (sudoku.get(row, col) == UNASSIGNED &&
                    sudoku.hasCandidate(row, col, digit))
                {
                    uint8_t br = (row / 3) * 3;
                    uint8_t bc = (col / 3) * 3;
                    if (count == 0) { lockedBoxRow = br; lockedBoxCol = bc; }
                    else if (br != lockedBoxRow || bc != lockedBoxCol)
                        lockedBoxRow = 255;
                    ++count;
                }

            if (count <= 1 || lockedBoxRow == 255) continue;

            for (uint8_t dr = 0; dr < 3; ++dr)
                for (uint8_t dc = 0; dc < 3; ++dc)
                {
                    uint8_t r = lockedBoxRow + dr;
                    uint8_t c = lockedBoxCol + dc;
                    if (r != row &&
                        sudoku.removeCandidate(r, c, digit))
                    {
                        progressed = true; ++localRemove;
                    }
                }
        }

    if (progressed) {
        logicalStats.data[LS_LOCKED_CLAIMING][0]++;
        logicalStats.data[LS_LOCKED_CLAIMING][1] += localRemove;
    }
    return progressed;
}

bool LogicalSolver::applyNakedPair(Sudoku& s)
{
    bool changed = false;
    uint32_t localRemove = 0;

    auto isPair = [](uint16_t m) { return std::popcount(m) == 2; };

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

                for (uint8_t c = 0; c < 9; ++c)
                    if (c != c1 && c != c2 &&
                        s.removeCandidatesMask(r, c, m))
                    {
                        changed = true; ++localRemove;
                    }
            }
        }

    if (changed) {
        logicalStats.data[LS_NAKED_PAIR][0]++;
        logicalStats.data[LS_NAKED_PAIR][1] += localRemove;
    }
    return changed;
}

bool LogicalSolver::applyHiddenPair(Sudoku& s)
{
    bool changed = false;
    uint32_t localRemove = 0;

    for (uint8_t r = 0; r < 9; ++r)
    {
        uint16_t pos[10] = {};

        for (uint8_t c = 0; c < 9; ++c)
            if (s.get(r, c) == UNASSIGNED)
            {
                uint16_t m = s.getCandidates(r, c);
                for (uint8_t d = 1; d <= 9; ++d)
                    if (m & bit(d)) pos[d] |= (1u << c);
            }

        for (uint8_t a = 1; a <= 8; ++a)
            for (uint8_t b = a + 1; b <= 9; ++b)
                if (pos[a] == pos[b] && std::popcount(pos[a]) == 2)
                {
                    uint16_t keep = bit(a) | bit(b);
                    for (uint8_t c = 0; c < 9; ++c)
                        if (pos[a] & (1u << c) &&
                            s.removeCandidatesMask(r, c, ~keep))
                        {
                            changed = true; ++localRemove;
                        }
                }
    }

    if (changed) {
        logicalStats.data[LS_HIDDEN_PAIR][0]++;
        logicalStats.data[LS_HIDDEN_PAIR][1] += localRemove;
    }
    return changed;
}
