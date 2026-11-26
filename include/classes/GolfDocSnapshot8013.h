// GolfDocSnapshot8013: layout view owned by golf_doc_turns_and_parameter_xml.cpp.
// Evidence: CGolfDoc_RestoreFirstPlayerSnapshot restores these fields from +0xC8; CGolfDoc_SaveFirstPlayerSnapshot copies them back. No virtual declarations or runtime objects.
#ifndef CLASSES_GOLFDOCSNAPSHOT8013_H
#define CLASSES_GOLFDOCSNAPSHOT8013_H

class GolfDocSnapshot8013 {
public:
    int playerId;                    // 0x0; CGolfDoc_RestoreFirstPlayerSnapshot restores these fields from +0xC8
    int unk_4;                       // 0x4; CGolfDoc_RestoreFirstPlayerSnapshot restores these fields from +0xC8
    int playerState;                 // 0x8; CGolfDoc_RestoreFirstPlayerSnapshot restores these fields from +0xC8
    unsigned char rankTieBreakOrder; // 0xC; CGolfDoc_RestoreFirstPlayerSnapshot restores these fields from +0xC8
    char pad_D[0x3];
    float previousBallPosition[3];      // 0x10; CGolfDoc_RestoreFirstPlayerSnapshot restores these fields from +0xC8
    float ballPosition[3];              // 0x1C; CGolfDoc_RestoreFirstPlayerSnapshot restores these fields from +0xC8
    unsigned char holeStrokeCounts[18]; // 0x28; CGolfDoc_RestoreFirstPlayerSnapshot restores these fields from +0xC8
    unsigned char holeResultValues[18]; // 0x3A; CGolfDoc_RestoreFirstPlayerSnapshot restores these fields from +0xC8
    union {
        unsigned char holePangTotals[72]; // snapshot byte-copy view
        int holePangWords[18];            // +4C: per-hole Pang words read/written during shot setup/undo
    };
    int unk_94;                      // 0x94; CGolfDoc_RestoreFirstPlayerSnapshot restores these fields from +0xC8
    int playerSlot;                  // 0x98; CGolfDoc_RestoreFirstPlayerSnapshot restores these fields from +0xC8
    unsigned char equippedItemCount; // 0x9C; CGolfDoc_RestoreFirstPlayerSnapshot restores these fields from +0xC8
    char pad_9D[0x3];
    int itemId;              // 0xA0; CGolfDoc_RestoreFirstPlayerSnapshot restores these fields from +0xC8
    unsigned char turnOrder; // 0xA4; CGolfDoc_RestoreFirstPlayerSnapshot restores these fields from +0xC8
    char pad_A5[0x3];
    float powerShotMeter;              // 0xA8; CGolfDoc_RestoreFirstPlayerSnapshot restores these fields from +0xC8
    short unk_AC;                      // 0xAC; CGolfDoc_RestoreFirstPlayerSnapshot restores these fields from +0xC8
    unsigned char unk_AE;              // 0xAE; CGolfDoc_RestoreFirstPlayerSnapshot restores these fields from +0xC8
    unsigned char timeoutsUsed;        // 0xAF; CGolfDoc_RestoreFirstPlayerSnapshot restores these fields from +0xC8
    unsigned char strokeLimitHitCount; // 0xB0; CGolfDoc_RestoreFirstPlayerSnapshot restores these fields from +0xC8
    char pad_B1[0x3];
    union {
        int unk_B4;
        int clearedShotWordB4;
    }; // +B4: cleared during shot setup
    int afterPuttPoseFlags; // 0xB8; CGolfDoc_RestoreFirstPlayerSnapshot restores these fields from +0xC8
    unsigned char unk_BC;   // 0xBC; CGolfDoc_RestoreFirstPlayerSnapshot restores these fields from +0xC8
    unsigned char unk_BD;   // 0xBD; CGolfDoc_RestoreFirstPlayerSnapshot restores these fields from +0xC8
    char pad_BE[0x2];
    union {
        int unk_C0;
        int accumulatedShotMilliseconds;
    }; // +C0
    unsigned char rankFlags;        // 0xC4; CGolfDoc_RestoreFirstPlayerSnapshot restores these fields from +0xC8
    unsigned char totalStrokeCount; // 0xC5; CGolfDoc_RestoreFirstPlayerSnapshot restores these fields from +0xC8
    char pad_C6[0x2];
    union {
        unsigned int roundResultWords[32]; // +C8: snapshot copy's original word view
        struct {
            int scoreRelativeToPar, pang, bonusPang; // +C8/+CC/+D0
            unsigned int completedHoleCount;         // +D4: incremented on hole completion
            int spinStat;                            // +D8: preserved statistic word
            unsigned int remainingResultWords[27];
        };
    };
    unsigned char unk_148;            // 0x148; CGolfDoc_RestoreFirstPlayerSnapshot restores these fields from +0xC8
    unsigned char unk_149;            // 0x149; CGolfDoc_RestoreFirstPlayerSnapshot restores these fields from +0xC8
    unsigned char rankTier;           // 0x14A; CGolfDoc_RestoreFirstPlayerSnapshot restores these fields from +0xC8
    unsigned char holeAwardFlags[18]; // 0x14B; CGolfDoc_RestoreFirstPlayerSnapshot restores these fields from +0xC8
    unsigned char unk_15D[15];        // 0x15D; CGolfDoc_RestoreFirstPlayerSnapshot restores these fields from +0xC8
    short unk_16C;                    // 0x16C; CGolfDoc_RestoreFirstPlayerSnapshot restores these fields from +0xC8
    unsigned char unk_16E;            // 0x16E; CGolfDoc_RestoreFirstPlayerSnapshot restores these fields from +0xC8
    char pad_16F[0x1];
    union {
        unsigned char unk_170[88]; // original snapshot byte-copy extent
        struct {
            char pad_170[8];
            int partyResult;
            char pad_17C[0x4C];
        };
    }; // +178: party result; interpretation depends on partyGameMode
};

#define OFF(f, o) typedef char GolfDocSnapshot8013_off_##f[((int)&((GolfDocSnapshot8013*)0)->f == (o)) ? 1 : -1]
OFF(playerId, 0x0);
OFF(unk_4, 0x4);
OFF(playerState, 0x8);
OFF(rankTieBreakOrder, 0xC);
OFF(previousBallPosition, 0x10);
OFF(ballPosition, 0x1C);
OFF(holeStrokeCounts, 0x28);
OFF(holeResultValues, 0x3A);
OFF(holePangTotals, 0x4C);
OFF(unk_94, 0x94);
OFF(playerSlot, 0x98);
OFF(equippedItemCount, 0x9C);
OFF(itemId, 0xA0);
OFF(turnOrder, 0xA4);
OFF(powerShotMeter, 0xA8);
OFF(unk_AC, 0xAC);
OFF(unk_AE, 0xAE);
OFF(timeoutsUsed, 0xAF);
OFF(strokeLimitHitCount, 0xB0);
OFF(unk_B4, 0xB4);
OFF(clearedShotWordB4, 0xB4);
OFF(accumulatedShotMilliseconds, 0xC0);
OFF(scoreRelativeToPar, 0xC8);
OFF(pang, 0xCC);
OFF(bonusPang, 0xD0);
OFF(completedHoleCount, 0xD4);
OFF(spinStat, 0xD8);
OFF(partyResult, 0x178);
OFF(holePangWords, 0x4C);
OFF(afterPuttPoseFlags, 0xB8);
OFF(unk_BC, 0xBC);
OFF(unk_BD, 0xBD);
OFF(unk_C0, 0xC0);
OFF(rankFlags, 0xC4);
OFF(totalStrokeCount, 0xC5);
OFF(roundResultWords, 0xC8);
OFF(unk_148, 0x148);
OFF(unk_149, 0x149);
OFF(rankTier, 0x14A);
OFF(holeAwardFlags, 0x14B);
OFF(unk_15D, 0x15D);
OFF(unk_16C, 0x16C);
OFF(unk_16E, 0x16E);
OFF(unk_170, 0x170);
#undef OFF

#endif
