#include <cuda_runtime.h>
#include <cstdint>

#define N 9
#define CELL_COUNT 81
#define UNASSIGNED 0
#define POS(r,c) ((r)*N + (c))
#define FULL_MASK 0x1FF

__device__ __forceinline__ uint16_t d_bit(uint8_t v)
{
    return (uint16_t)(1u << (v - 1));
}

__device__ __forceinline__ bool d_single(uint16_t m)
{
    return m && ((m & (m - 1)) == 0);
}

__device__ __forceinline__ uint8_t d_extract(uint16_t m)
{
    return (uint8_t)__ffs((int)m);
}

__device__ void d_recomputeCandidates(uint8_t* g, uint16_t* c)
{
    for (int i = 0; i < CELL_COUNT; ++i)
        c[i] = (g[i] == UNASSIGNED) ? FULL_MASK : 0;

    for (int r = 0; r < 9; ++r)
        for (int col = 0; col < 9; ++col)
        {
            int i = POS(r, col);
            uint8_t v = g[i];
            if (v == UNASSIGNED) continue;

            uint16_t kill = ~d_bit(v);

            for (int k = 0; k < 9; ++k)
            {
                if (g[POS(r, k)] == UNASSIGNED) c[POS(r, k)] &= kill;
                if (g[POS(k, col)] == UNASSIGNED) c[POS(k, col)] &= kill;
            }

            int br = (r / 3) * 3;
            int bc = (col / 3) * 3;
            for (int dr = 0; dr < 3; ++dr)
                for (int dc = 0; dc < 3; ++dc)
                {
                    int rr = br + dr;
                    int cc = bc + dc;
                    if (g[POS(rr, cc)] == UNASSIGNED)
                        c[POS(rr, cc)] &= kill;
                }
        }
}

__device__ void d_update(uint8_t* g, uint16_t* c, int r, int col, uint8_t v)
{
    uint16_t kill = ~d_bit(v);

    for (int i = 0; i < 9; ++i)
    {
        if (g[POS(r, i)] == UNASSIGNED) c[POS(r, i)] &= kill;
        if (g[POS(i, col)] == UNASSIGNED) c[POS(i, col)] &= kill;
    }

    int br = (r / 3) * 3;
    int bc = (col / 3) * 3;
    for (int dr = 0; dr < 3; ++dr)
        for (int dc = 0; dc < 3; ++dc)
        {
            int rr = br + dr;
            int cc = bc + dc;
            if (g[POS(rr, cc)] == UNASSIGNED)
                c[POS(rr, cc)] &= kill;
        }
}

__device__ bool d_nakedSingle(uint8_t* g, uint16_t* c)
{
    bool progressed = false;

    for (int i = 0; i < CELL_COUNT; ++i)
    {
        if (g[i] != UNASSIGNED) continue;
        if (!d_single(c[i])) continue;

        uint8_t v = d_extract(c[i]);
        g[i] = v;
        c[i] = 0;
        d_update(g, c, i / 9, i % 9, v);
        progressed = true;
    }
    return progressed;
}

__device__ bool d_hiddenSingle(uint8_t* g, uint16_t* c)
{
    bool progressed = false;

    for (int r = 0; r < 9; ++r)
        for (uint8_t d = 1; d <= 9; ++d)
        {
            int cnt = 0, tc = -1;
            uint16_t m = d_bit(d);

            for (int col = 0; col < 9 && cnt <= 1; ++col)
            {
                int i = POS(r, col);
                if (g[i] == UNASSIGNED && (c[i] & m))
                {
                    tc = col;
                    ++cnt;
                }
            }

            if (cnt == 1)
            {
                int i = POS(r, tc);
                g[i] = d;
                c[i] = 0;
                d_update(g, c, r, tc, d);
                progressed = true;
            }
        }

    for (int col = 0; col < 9; ++col)
        for (uint8_t d = 1; d <= 9; ++d)
        {
            int cnt = 0, tr = -1;
            uint16_t m = d_bit(d);

            for (int r = 0; r < 9 && cnt <= 1; ++r)
            {
                int i = POS(r, col);
                if (g[i] == UNASSIGNED && (c[i] & m))
                {
                    tr = r;
                    ++cnt;
                }
            }

            if (cnt == 1)
            {
                int i = POS(tr, col);
                g[i] = d;
                c[i] = 0;
                d_update(g, c, tr, col, d);
                progressed = true;
            }
        }

    for (int br = 0; br < 9; br += 3)
        for (int bc = 0; bc < 9; bc += 3)
            for (uint8_t d = 1; d <= 9; ++d)
            {
                int cnt = 0, tr = -1, tc = -1;
                uint16_t m = d_bit(d);

                for (int dr = 0; dr < 3 && cnt <= 1; ++dr)
                    for (int dc = 0; dc < 3 && cnt <= 1; ++dc)
                    {
                        int r = br + dr;
                        int c0 = bc + dc;
                        int i = POS(r, c0);
                        if (g[i] == UNASSIGNED && (c[i] & m))
                        {
                            tr = r; tc = c0;
                            ++cnt;
                        }
                    }

                if (cnt == 1)
                {
                    int i = POS(tr, tc);
                    g[i] = d;
                    c[i] = 0;
                    d_update(g, c, tr, tc, d);
                    progressed = true;
                }
            }

    return progressed;
}

__device__ bool d_lockedPointing(uint8_t* g, uint16_t* c)
{
    bool progressed = false;

    for (int br = 0; br < 9; br += 3)
        for (int bc = 0; bc < 9; bc += 3)
            for (uint8_t d = 1; d <= 9; ++d)
            {
                int count = 0;
                int lockR = -1, lockC = -1;
                uint16_t m = d_bit(d);

                for (int dr = 0; dr < 3; ++dr)
                    for (int dc = 0; dc < 3; ++dc)
                    {
                        int r = br + dr;
                        int c0 = bc + dc;
                        int i = POS(r, c0);
                        if (g[i] == UNASSIGNED && (c[i] & m))
                        {
                            if (count == 0) { lockR = r; lockC = c0; }
                            else {
                                if (r != lockR) lockR = -1;
                                if (c0 != lockC) lockC = -1;
                            }
                            ++count;
                        }
                    }

                if (count <= 1) continue;

                if (lockR != -1)
                    for (int col = 0; col < 9; ++col)
                        if (col < bc || col >= bc + 3)
                        {
                            int i = POS(lockR, col);
                            if (g[i] == UNASSIGNED && (c[i] & m))
                            {
                                c[i] &= ~m;
                                progressed = true;
                            }
                        }

                if (lockC != -1)
                    for (int r = 0; r < 9; ++r)
                        if (r < br || r >= br + 3)
                        {
                            int i = POS(r, lockC);
                            if (g[i] == UNASSIGNED && (c[i] & m))
                            {
                                c[i] &= ~m;
                                progressed = true;
                            }
                        }
            }

    return progressed;
}

__device__ bool d_lockedClaiming(uint8_t* g, uint16_t* c)
{
    bool progressed = false;

    for (int r = 0; r < 9; ++r)
        for (uint8_t d = 1; d <= 9; ++d)
        {
            int count = 0;
            int box = -1;
            uint16_t m = d_bit(d);

            for (int col = 0; col < 9; ++col)
            {
                int i = POS(r, col);
                if (g[i] == UNASSIGNED && (c[i] & m))
                {
                    int b = col / 3;
                    if (count == 0) box = b;
                    else if (b != box) box = -1;
                    ++count;
                }
            }

            if (count <= 1 || box == -1) continue;

            int bc = box * 3;
            int br = (r / 3) * 3;

            for (int dr = 0; dr < 3; ++dr)
                for (int dc = 0; dc < 3; ++dc)
                {
                    int rr = br + dr;
                    int cc = bc + dc;
                    if (rr != r)
                    {
                        int i = POS(rr, cc);
                        if (g[i] == UNASSIGNED && (c[i] & m))
                        {
                            c[i] &= ~m;
                            progressed = true;
                        }
                    }
                }
        }

    return progressed;
}

__device__ bool d_nakedPair(uint8_t* g, uint16_t* c)
{
    bool progressed = false;

    for (int r = 0; r < 9; ++r)
        for (int c1 = 0; c1 < 8; ++c1)
        {
            int i1 = POS(r, c1);
            if (g[i1] != UNASSIGNED) continue;
            uint16_t m = c[i1];
            if (__popc(m) != 2) continue;

            for (int c2 = c1 + 1; c2 < 9; ++c2)
            {
                int i2 = POS(r, c2);
                if (g[i2] != UNASSIGNED) continue;
                if (c[i2] != m) continue;

                for (int c3 = 0; c3 < 9; ++c3)
                    if (c3 != c1 && c3 != c2)
                    {
                        int i = POS(r, c3);
                        if (g[i] == UNASSIGNED && (c[i] & m))
                        {
                            c[i] &= ~m;
                            progressed = true;
                        }
                    }
            }
        }

    for (int col = 0; col < 9; ++col)
        for (int r1 = 0; r1 < 8; ++r1)
        {
            int i1 = POS(r1, col);
            if (g[i1] != UNASSIGNED) continue;
            uint16_t m = c[i1];
            if (__popc(m) != 2) continue;

            for (int r2 = r1 + 1; r2 < 9; ++r2)
            {
                int i2 = POS(r2, col);
                if (g[i2] != UNASSIGNED) continue;
                if (c[i2] != m) continue;

                for (int r3 = 0; r3 < 9; ++r3)
                    if (r3 != r1 && r3 != r2)
                    {
                        int i = POS(r3, col);
                        if (g[i] == UNASSIGNED && (c[i] & m))
                        {
                            c[i] &= ~m;
                            progressed = true;
                        }
                    }
            }
        }

    for (int br = 0; br < 9; br += 3)
        for (int bc = 0; bc < 9; bc += 3)
            for (int p1 = 0; p1 < 8; ++p1)
            {
                int r1 = br + p1 / 3;
                int c1 = bc + p1 % 3;
                int i1 = POS(r1, c1);
                if (g[i1] != UNASSIGNED) continue;
                uint16_t m = c[i1];
                if (__popc(m) != 2) continue;

                for (int p2 = p1 + 1; p2 < 9; ++p2)
                {
                    int r2 = br + p2 / 3;
                    int c2 = bc + p2 % 3;
                    int i2 = POS(r2, c2);
                    if (g[i2] != UNASSIGNED) continue;
                    if (c[i2] != m) continue;

                    for (int p3 = 0; p3 < 9; ++p3)
                        if (p3 != p1 && p3 != p2)
                        {
                            int rr = br + p3 / 3;
                            int cc = bc + p3 % 3;
                            int i = POS(rr, cc);
                            if (g[i] == UNASSIGNED && (c[i] & m))
                            {
                                c[i] &= ~m;
                                progressed = true;
                            }
                        }
                }
            }

    return progressed;
}

__device__ bool d_hiddenPair(uint8_t* g, uint16_t* c)
{
    bool progressed = false;

    for (int r = 0; r < 9; ++r)
    {
        uint16_t pos[10] = {};
        for (int col = 0; col < 9; ++col)
        {
            int i = POS(r, col);
            if (g[i] != UNASSIGNED) continue;
            uint16_t m = c[i];
            for (uint8_t d = 1; d <= 9; ++d)
                if (m & d_bit(d)) pos[d] |= (1u << col);
        }

        for (uint8_t a = 1; a <= 8; ++a)
            for (uint8_t b = a + 1; b <= 9; ++b)
                if (pos[a] == pos[b] && __popc(pos[a]) == 2)
                {
                    uint16_t keep = d_bit(a) | d_bit(b);
                    for (int col = 0; col < 9; ++col)
                        if (pos[a] & (1u << col))
                        {
                            int i = POS(r, col);
                            if (c[i] & ~keep)
                            {
                                c[i] &= keep;
                                progressed = true;
                            }
                        }
                }
    }

    for (int col = 0; col < 9; ++col)
    {
        uint16_t pos[10] = {};
        for (int r = 0; r < 9; ++r)
        {
            int i = POS(r, col);
            if (g[i] != UNASSIGNED) continue;
            uint16_t m = c[i];
            for (uint8_t d = 1; d <= 9; ++d)
                if (m & d_bit(d)) pos[d] |= (1u << r);
        }

        for (uint8_t a = 1; a <= 8; ++a)
            for (uint8_t b = a + 1; b <= 9; ++b)
                if (pos[a] == pos[b] && __popc(pos[a]) == 2)
                {
                    uint16_t keep = d_bit(a) | d_bit(b);
                    for (int r = 0; r < 9; ++r)
                        if (pos[a] & (1u << r))
                        {
                            int i = POS(r, col);
                            if (c[i] & ~keep)
                            {
                                c[i] &= keep;
                                progressed = true;
                            }
                        }
                }
    }

    for (int br = 0; br < 9; br += 3)
        for (int bc = 0; bc < 9; bc += 3)
        {
            uint16_t pos[10] = {};
            for (int p = 0; p < 9; ++p)
            {
                int r = br + p / 3;
                int col = bc + p % 3;
                int i = POS(r, col);
                if (g[i] != UNASSIGNED) continue;
                uint16_t m = c[i];
                for (uint8_t d = 1; d <= 9; ++d)
                    if (m & d_bit(d)) pos[d] |= (1u << p);
            }

            for (uint8_t a = 1; a <= 8; ++a)
                for (uint8_t b = a + 1; b <= 9; ++b)
                    if (pos[a] == pos[b] && __popc(pos[a]) == 2)
                    {
                        uint16_t keep = d_bit(a) | d_bit(b);
                        for (int p = 0; p < 9; ++p)
                            if (pos[a] & (1u << p))
                            {
                                int r = br + p / 3;
                                int col = bc + p % 3;
                                int i = POS(r, col);
                                if (c[i] & ~keep)
                                {
                                    c[i] &= keep;
                                    progressed = true;
                                }
                            }
                    }
        }

    return progressed;
}

__device__ bool d_nakedTriple(uint8_t* g, uint16_t* c)
{
    bool progressed = false;

    auto handleUnit = [&](const int idx[9]) {
        for (int a = 0; a < 7; ++a)
            for (int b = a + 1; b < 8; ++b)
                for (int d = b + 1; d < 9; ++d)
                {
                    int i1 = idx[a], i2 = idx[b], i3 = idx[d];
                    if (g[i1] != UNASSIGNED || g[i2] != UNASSIGNED || g[i3] != UNASSIGNED) continue;

                    uint16_t m1 = c[i1];
                    uint16_t m2 = c[i2];
                    uint16_t m3 = c[i3];
                    uint16_t uni = m1 | m2 | m3;
                    if (__popc(uni) != 3) continue;

                    for (int k = 0; k < 9; ++k)
                    {
                        int i = idx[k];
                        if (i == i1 || i == i2 || i == i3) continue;
                        if (g[i] == UNASSIGNED && (c[i] & uni))
                        {
                            c[i] &= ~uni;
                            progressed = true;
                        }
                    }
                }
        };

    for (int r = 0; r < 9; ++r) {
        int idx[9];
        for (int c0 = 0; c0 < 9; ++c0) idx[c0] = r * 9 + c0;
        handleUnit(idx);
    }

    for (int col = 0; col < 9; ++col) {
        int idx[9];
        for (int r = 0; r < 9; ++r) idx[r] = r * 9 + col;
        handleUnit(idx);
    }

    for (int br = 0; br < 9; br += 3)
        for (int bc = 0; bc < 9; bc += 3) {
            int idx[9], p = 0;
            for (int dr = 0; dr < 3; ++dr)
                for (int dc = 0; dc < 3; ++dc)
                    idx[p++] = (br + dr) * 9 + (bc + dc);
            handleUnit(idx);
        }

    return progressed;
}

__device__ bool d_hiddenTriple(uint8_t* g, uint16_t* c)
{
    bool progressed = false;

    auto handleUnit = [&](const int idx[9]) {
        uint16_t pos[10] = {};

        for (int k = 0; k < 9; ++k)
        {
            int i = idx[k];
            if (g[i] != UNASSIGNED) continue;
            uint16_t m = c[i];
            for (uint8_t d = 1; d <= 9; ++d)
                if (m & d_bit(d))
                    pos[d] |= (1u << k);
        }

        for (uint8_t a = 1; a <= 7; ++a)
            for (uint8_t b = a + 1; b <= 8; ++b)
                for (uint8_t d = b + 1; d <= 9; ++d)
                {
                    uint16_t cells = pos[a] | pos[b] | pos[d];
                    if (__popc(cells) != 3) continue;
                    if (pos[a] == 0 || pos[b] == 0 || pos[d] == 0) continue;
                    if ((pos[a] & cells) != pos[a]) continue;
                    if ((pos[b] & cells) != pos[b]) continue;
                    if ((pos[d] & cells) != pos[d]) continue;

                    uint16_t keep = d_bit(a) | d_bit(b) | d_bit(d);

                    for (int k = 0; k < 9; ++k)
                        if (cells & (1u << k))
                        {
                            int i = idx[k];
                            if (c[i] & ~keep)
                            {
                                c[i] &= keep;
                                progressed = true;
                            }
                        }
                }
        };

    for (int r = 0; r < 9; ++r) {
        int idx[9];
        for (int c0 = 0; c0 < 9; ++c0) idx[c0] = r * 9 + c0;
        handleUnit(idx);
    }

    for (int col = 0; col < 9; ++col) {
        int idx[9];
        for (int r = 0; r < 9; ++r) idx[r] = r * 9 + col;
        handleUnit(idx);
    }

    for (int br = 0; br < 9; br += 3)
        for (int bc = 0; bc < 9; bc += 3) {
            int idx[9], p = 0;
            for (int dr = 0; dr < 3; ++dr)
                for (int dc = 0; dc < 3; ++dc)
                    idx[p++] = (br + dr) * 9 + (bc + dc);
            handleUnit(idx);
        }

    return progressed;
}

__global__ void logicalKernel(uint8_t* grids, uint16_t* cands, int count)
{
    int tid = blockIdx.x * blockDim.x + threadIdx.x;
    if (tid >= count) return;

    uint8_t* g = grids + tid * CELL_COUNT;
    uint16_t* c = cands + tid * CELL_COUNT;

    d_recomputeCandidates(g, c);

    bool progress;
    do {
        progress = false;
        progress |= d_nakedSingle(g, c);
        progress |= d_hiddenSingle(g, c);
        progress |= d_lockedPointing(g, c);
        progress |= d_lockedClaiming(g, c);
        progress |= d_nakedPair(g, c);
        progress |= d_hiddenPair(g, c);
        progress |= d_nakedTriple(g, c);
        progress |= d_hiddenTriple(g, c);
    } while (progress);
}

extern "C" __declspec(dllexport)
void runCudaLogicalBasic(uint8_t* h_grids, uint16_t* h_cands, int sudokuCount)
{
    uint8_t* d_g = nullptr;
    uint16_t* d_c = nullptr;

    size_t gb = (size_t)sudokuCount * CELL_COUNT;
    size_t cb = gb * sizeof(uint16_t);

    cudaMalloc(&d_g, gb);
    cudaMalloc(&d_c, cb);

    cudaMemcpy(d_g, h_grids, gb, cudaMemcpyHostToDevice);
    cudaMemcpy(d_c, h_cands, cb, cudaMemcpyHostToDevice);

    int threads = 256;
    int blocks = (sudokuCount + threads - 1) / threads;

    logicalKernel << <blocks, threads >> > (d_g, d_c, sudokuCount);
    cudaDeviceSynchronize();

    cudaMemcpy(h_grids, d_g, gb, cudaMemcpyDeviceToHost);
    cudaMemcpy(h_cands, d_c, cb, cudaMemcpyDeviceToHost);

    cudaFree(d_g);
    cudaFree(d_c);
}
