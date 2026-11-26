// gtl::nw4r::detail::StrmPlayer: streaming playback state of one stream player. It is embedded in the stream
// object; Object (gtl_strm_player_streaming.cpp) reaches it through the node at +0x74. The methods are defined in
// src/engine/gtl/gtl_strm_player_streaming.cpp and gtl_sound_stream_player.cpp.
#ifndef CLASSES_GTLSTRMPLAYER_H
#define CLASSES_GTLSTRMPLAYER_H

struct Reader;

// Intrusive list link used by the stream player's active list and read queues.
struct Node {
    Node* next;
    Node* prev;
};

// Intrusive list with a count and a sentinel head.
struct List {
    unsigned int count;
    Node head;
};

namespace gtl {
namespace nw4r {
namespace detail {

class StrmPlayer {
public:
    // Destroy the 32 channel entries and both read lists; frees this object when freeMemory > 0. Returns this.
    void* Destroy(int freeMemory);
    // Convert the seek position to samples, wrap looping streams, and return block and in-block offsets.
    int ResolveStartBlock(int* blockIndex, unsigned int* sampleOffset, int* completedLoops);
    // Resolve the seek position and configure bounded circular buffering before acquiring channels.
    int SetupStreamingBuffers(void* arg);
    // Set voice loop/end addresses at a file boundary, including the decoder loop context.
    void ConfigureBlockBoundaryAddresses(int index);
    // Advance file and circular-buffer playback indices and update loop ranges at either boundary.
    void AdvanceStreamBlock();
    // Voice callback: clear the voice on every terminal event; release it explicitly for reasons 0 and 1.
    static void OnVoiceTerminalEvent(void* voice, int mode, StrmPlayer* player);

    char padding00[8];
    float volumeScale, pitchScale;
    float pan, surroundPan, panOffset, surroundPanOffset;
    float lowPassFilterOffset;
    int unknown24;
    int outputLineFlags;
    float mainOutputVolume, mainSend;
    char padding34[0x3c];
    unsigned char biquadFilter;
    char padding71[3];
    Node activePlayerNode;
    unsigned char sampleFormat;
    unsigned char loopEnabled;
    unsigned char sourceChannelCount;
    char padding7F;
    unsigned short sampleRate;
    unsigned short blockHeaderBytes;
    int loopStartSample;
    unsigned int totalSamples;
    int fileDataOffset;
    int fileBlockCount;
    int blockBytes;
    int samplesPerBlock;
    int lastBlockBytes;
    unsigned int lastBlockSamples;
    int lastBlockAlignedBytes;
    char paddingA8[4];
    unsigned char flags[13];
    char paddingB9[3];
    int loopCount;
    int preloadedBlocks;
    int initialReadBlocks;
    int bufferBlockBytes;
    int bufferBlockCapacity;
    int readAheadBlocks;
    int nextCycleBlocks;
    int nextReadBufferBlock;
    int nextReadFileBlock;
    int currentCycleBlocks;
    int playingBufferBlock;
    int playingFileBlock;
    int loopStartBlock;
    int lastFileBlock;
    int seekUnit;
    int seekPosition;
    List freeReadPackets;
    List pendingReadPackets;
    char channelEntries[0x20][0x1C];
    void* bufferPool;
    Reader* reader;
    int readerUserData;
    void* voice;
    int voiceChannels;
    void* voiceUserData;
    void* channelBuffer;
    int channelBufferBytes;
    char channelDecoderState[0x28];
    unsigned short channelLoopContext[4];
    int secondChannelBuffer;
    int secondChannelBufferBytes;
    char secondChannelDecoderState[0x30];
    unsigned short pendingLoopPredictorScale[2];
};

} // namespace detail
} // namespace nw4r
} // namespace gtl

#endif // CLASSES_GTLSTRMPLAYER_H
