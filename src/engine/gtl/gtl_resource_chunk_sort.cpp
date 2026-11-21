// gtl resource loading: sort contiguous 16-byte resource chunks by unsigned
// file offset (+0xC). The other three words travel with the key unchanged.
// Mod lookup: small-range cutoff is 20 records below; pivot sampling uses
// s_resourceChunkPivotSampleCycle and SortThreeByOffset (gtl_resource_chunk_sort3.cpp). No gameplay tuning knobs.
// Translation unit 0x803D7914-0x803D7E18 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

struct ResChunk {
    unsigned int unk0;
    unsigned int unk4;
    unsigned int unk8;
    unsigned int offset;
};

// Shared deterministic sampling cycle, not RNG. Normal cycle values lie in
// [-4,4]; two successive values perturb the quarter/three-quarter samples.
// C++ signed % is deliberate: negative residues move a sample toward the start.
// Sorting mutates this global (initial value -4 at 805DC154); callers must
// serialize concurrent invocations. It changes sample choice, not sort direction.
extern int s_resourceChunkPivotSampleCycle;

void gtl_ResourceChunks_SortThreeByOffset(ResChunk** a, ResChunk** b, ResChunk** c, void* ctx);

#pragma use_lmw_stmw on
// Sort [*pBegin,*pEnd) in place, ascending offset, unstable for equal keys.
// Both pointer slots are required, distinct, writable and outside the records;
// both bounds belong to the same contiguous array and end is one-past-last.
// Record count must fit signed int and n*3 without signed overflow (automatically
// true for a 16-byte array that fits Wii address space). Empty/single ranges return.
// Payloads are copied, not allocated/freed; callers retain all resource ownership.
// The slots themselves are working cursors: this function may ADVANCE *pBegin
// or RETREAT *pEnd and does not restore them. Pass local bound copies when the
// original range is needed after sorting. ctx is only forwarded to the sampler
// and recursive calls; the sampler currently ignores that ABI argument.
// No error return or range validation. Normal sampling-cycle state is required.
// Phase map: <=20 selection sort; otherwise sample/partition, then either skip
// the equal-minimum run or recurse on the smaller partition and loop on larger.
void gtl_ResourceChunks_SortByOffsetRecursive(ResChunk** pBegin, ResChunk** pEnd, void* ctx)
{
    ResChunk* last;
    for (;;) {
        int n = *pEnd - *pBegin;
        if (n <= 1) {
            return;
        }
        if (n <= 20) {
            // Finish the active range by selection sort. cur separates the
            // sorted prefix from the suffix searched for its next minimum.
            // Volatile bound reads preserve the original loads and alias behavior.
            ResChunk* end = *(ResChunk* volatile*)pEnd;
            ResChunk* cur = *(ResChunk* volatile*)pBegin;
            if (cur != end) {
                last = end - 1;
                if (cur != last) {
                    while (cur != last) {
                        ResChunk* min = cur;
                        if (cur != end) {
                            for (ResChunk* p = cur + 1; p != end; p++) {
                                if (p->offset < min->offset) {
                                    min = p;
                                }
                            }
                        }
                        if (min != cur) {
                            ResChunk tmp = *min;
                            *min = *cur;
                            *cur = tmp;
                        }
                        cur++;
                    }
                }
            }
            return;
        }

        // Sample two interior records and the last record. SortThreeByOffset
        // rearranges whole sample records so the median key is at last; the
        // first interior sample provides a lower sentinel for the reverse scan.
        // Quarter indices plus signed residues stay in range for n > 20.
        int v = s_resourceChunkPivotSampleCycle;
        ResChunk* m1 = *(ResChunk* volatile*)pBegin + (n / 4 + v % 5);
        int k = v + 1;
        if (k >= 5) {
            k = -4;
        }
        int next = k + 1;
        // Advance twice with wrap through negative values. Preserve the two
        // stores: the helper/recursive calls observe this shared cycle.
        s_resourceChunkPivotSampleCycle = next;
        int t = n * 3 / 4;
        ResChunk* m2 = *pBegin + (t + k % 5);
        if (next >= 5) {
            s_resourceChunkPivotSampleCycle = k - 8;
        }
        last = *pEnd - 1;
        ResChunk* m3 = last;
        gtl_ResourceChunks_SortThreeByOffset(&m1, &m2, &m3, ctx);

        // Partition invariant: [begin,p) < pivot and (q,last) >= pivot.
        // p stops on >= (including equals); q skips >= and stops on <.
        // Swapping and incrementing p ensures progress even with duplicate keys.
        ResChunk* p = *pBegin;
        while (p->offset < last->offset) {
            p++;
        }
        ResChunk* q = last;
        do {
            q--;
            if (q == p) {
                break;
            }
        } while (q->offset >= last->offset);
        if (p < q) {
            ResChunk tmp = *p;
            *p = *q;
            p++;
            *q = tmp;
            for (;;) {
                while (p->offset < last->offset) {
                    p++;
                }
                do {
                    q--;
                } while (q->offset >= last->offset);
                if (q - p <= 0) {
                    break;
                }
                ResChunk tmp2 = *p;
                *p = *q;
                p++;
                *q = tmp2;
            }
        }

        if (p == *pBegin) {
            // No strictly smaller key: move the pivot to begin, then partition
            // the remainder into <= pivot and > pivot. Since pivot is minimal,
            // the skipped <= prefix consists of equal keys. Initial p++ consumes
            // at least one record, including the all-equal case; looping cannot
            // repeat the same active range.
            ResChunk tmp = *p;
            ResChunk* lp = *(ResChunk* volatile*)pEnd - 1;
            *p = *last;
            p++;
            *last = tmp;
            if (!((*pBegin)->offset < lp->offset)) {
                ResChunk* e = *pEnd;
                while (p != e && (*pBegin)->offset >= p->offset) {
                    p++;
                }
                if (p < lp) {
                    ResChunk tmp2 = *p;
                    *p = *lp;
                    *lp = tmp2;
                }
            }
            if (p < lp) {
                for (;;) {
                    while ((*pBegin)->offset >= p->offset) {
                        p++;
                    }
                    do {
                        lp--;
                    } while ((*pBegin)->offset < lp->offset);
                    if (lp - p <= 0) {
                        break;
                    }
                    ResChunk tmp3 = *p;
                    *p = *lp;
                    p++;
                    *lp = tmp3;
                }
            }
            *pBegin = p;
        } else {
            // Both partitions are nonempty: p is beyond begin and cannot pass
            // the pivot at end-1. Recursing on the smaller one bounds stack use;
            // replacing one bound with p strictly shrinks the iterative range.
            int left = p - *pBegin;
            int right = *pEnd - p;
            if (left < right) {
                ResChunk* t1 = *pBegin;
                ResChunk* t2 = p;
                gtl_ResourceChunks_SortByOffsetRecursive(&t1, &t2, ctx);
                *pBegin = p;
            } else {
                ResChunk* t1 = p;
                ResChunk* t2 = *pEnd;
                gtl_ResourceChunks_SortByOffsetRecursive(&t1, &t2, ctx);
                *pEnd = p;
            }
        }
    }
}

#pragma use_lmw_stmw reset

} // extern "C"
