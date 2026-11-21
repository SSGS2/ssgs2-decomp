// Runtime shot payloads, shared by the rule and golf document layouts.
// Evidence: round-flow reset/undo, launch producers and hole scoring;
#ifndef GAME_GOLF_SHOT_TYPES_H
#define GAME_GOLF_SHOT_TYPES_H

#include "game/wvector.h"

// Standalone payload copied into blocks of 16 by the shot-history container.
// Opaque player bytes and copied reserved words deliberately retain neutral names.
struct ShotCheckpoint {
    WVector playerPosition;
    float headingRadians;
    unsigned char recoveryRequired, unk_PlayerAE;
    char pad_012[2];
    float powerShotMeter;
    int pang, bonusPang, holePang;
    int reserved[8];
    char clubName[4];
    unsigned char holeStrokes, holeResult, unk_PlayerBC, pad_04B;
    int partyResult, powerMeterCost;
};

// Storage only: the existing container owns allocation and lifetime management.
// RestorePreviousShot proves the block-map origin, occupied count and capacity.
// The rule's legacy shotHistoryCount keeps its signed access at the same address.
struct ShotHistoryStorage {
    unsigned int start, size;
    void* map;
    unsigned int mapStart, blocks, mapSize;
};

struct ShotHoleRecord {
    WVector teePosition, pinPosition;
    unsigned char par, unk_19;
    char pad_01A[6];
};

typedef char ShotCheckpointPayloadSize[sizeof(ShotCheckpoint) == 0x54 ? 1 : -1];
typedef char ShotHistoryStorageSize[sizeof(ShotHistoryStorage) == 0x18 ? 1 : -1];
typedef char ShotHoleRecordPayloadSize[sizeof(ShotHoleRecord) == 0x20 ? 1 : -1];
#define SHOT_PAYLOAD_OFF(t, f, o) typedef char ShotPayload_##t##_##f[((int)&((t*)0)->f == (o)) ? 1 : -1]
SHOT_PAYLOAD_OFF(ShotCheckpoint, playerPosition, 0x00);
SHOT_PAYLOAD_OFF(ShotCheckpoint, headingRadians, 0x0C);
SHOT_PAYLOAD_OFF(ShotCheckpoint, recoveryRequired, 0x10);
SHOT_PAYLOAD_OFF(ShotCheckpoint, unk_PlayerAE, 0x11);
SHOT_PAYLOAD_OFF(ShotCheckpoint, powerShotMeter, 0x14);
SHOT_PAYLOAD_OFF(ShotCheckpoint, pang, 0x18);
SHOT_PAYLOAD_OFF(ShotCheckpoint, bonusPang, 0x1C);
SHOT_PAYLOAD_OFF(ShotCheckpoint, holePang, 0x20);
SHOT_PAYLOAD_OFF(ShotCheckpoint, reserved, 0x24);
SHOT_PAYLOAD_OFF(ShotCheckpoint, clubName, 0x44);
SHOT_PAYLOAD_OFF(ShotCheckpoint, holeStrokes, 0x48);
SHOT_PAYLOAD_OFF(ShotCheckpoint, holeResult, 0x49);
SHOT_PAYLOAD_OFF(ShotCheckpoint, unk_PlayerBC, 0x4A);
SHOT_PAYLOAD_OFF(ShotCheckpoint, partyResult, 0x4C);
SHOT_PAYLOAD_OFF(ShotCheckpoint, powerMeterCost, 0x50);
SHOT_PAYLOAD_OFF(ShotHistoryStorage, size, 0x04);
SHOT_PAYLOAD_OFF(ShotHistoryStorage, map, 0x08);
SHOT_PAYLOAD_OFF(ShotHistoryStorage, mapStart, 0x0C);
SHOT_PAYLOAD_OFF(ShotHistoryStorage, blocks, 0x10);
SHOT_PAYLOAD_OFF(ShotHistoryStorage, mapSize, 0x14);
SHOT_PAYLOAD_OFF(ShotHoleRecord, pinPosition, 0x0C);
SHOT_PAYLOAD_OFF(ShotHoleRecord, par, 0x18);
#undef SHOT_PAYLOAD_OFF
#endif
