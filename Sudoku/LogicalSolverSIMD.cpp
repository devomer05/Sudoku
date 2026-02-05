#include "LogicalSolverSIMD.h"
#include "simd_utils.h"

bool LogicalSolverSIMD::applyNakedSingle(Sudoku& s)
{
    const uint16_t* cand = s.candidatesData();

    uint8_t idxList[81];
    uint8_t valList[81];
    int count = 0;

    // SIMD part
    for (int i = 0; i + 15 < 81; i += 16)
    {
        __m256i v = load_u16(&cand[i]);
        __m256i singles = mask_single_bit_epi16(v);

        if (!any_lane(singles))
            continue;

        // scan lanes
        for (int lane = 0; lane < 16; lane++)
        {
            int idx = i + lane;
            if (idx >= 81)
                break;

            uint16_t m = cand[idx];
            if (!singleMask(m))
                continue;

            uint8_t r = idx / 9;
            uint8_t c = idx % 9;

            if (s.get(r, c) != UNASSIGNED)
                continue;

            idxList[count] = idx;
            valList[count] = extractSingleValue(m);
            count++;
        }
    }

    // tail
    uint16_t m = cand[80];
    if (singleMask(m) && s.get(8, 8) == UNASSIGNED)
    {
        idxList[count] = 80;
        valList[count] = extractSingleValue(m);
        count++;
    }

    if (count == 0)
        return false;

    logicalStats.data[LS_NAKED_SINGLE][0] += count; // hit
    logicalStats.data[LS_NAKED_SINGLE][1] += count; // effect

    for (int k = 0; k < count; k++)
    {
        int idx = idxList[k];
        uint8_t r = idx / 9;
        uint8_t c = idx % 9;
        s.set(r, c, valList[k]);
        s.updateCandidatesAfterSet(r, c, valList[k]);
    }

    return true;
}
