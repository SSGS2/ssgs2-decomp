// Each vertex stores W-1 explicit weights; the last weight is 1 - their sum.
// Bone indices use four bytes, and each matrix occupies 64 bytes in the palette.
template <int WeightCount, int TexCount> struct SkinSourceVertex {
    float position[3];
    float weights[WeightCount - 1];
    float normal[3];
    unsigned char boneIndices[4];
    float texCoords[TexCount][2];
    enum {
        kWeightsOffset = sizeof(float) * 3,
        kNormalOffset = sizeof(float) * (WeightCount + 2),
        kBoneIndicesOffset = sizeof(float) * (WeightCount + 5),
        kTexCoordsOffset = kBoneIndicesOffset + 4
    };
};
template <int TexCount> struct SkinOutputVertex {
    float position[3];
    float normal[3];
    float texCoords[TexCount][2];
    enum {
        kNormalOffset = sizeof(float) * 3,
        kTexCoordsOffset = sizeof(float) * 6
    };
};
// Guard the named offsets used by the compiler-sensitive byte cursor loops.
template <int WeightCount> struct SkinSourceLayoutCheck {
    typedef SkinSourceVertex<WeightCount, 3> Vertex;
    char offsets[((int)&((Vertex*)0)->weights == Vertex::kWeightsOffset &&
                  (int)&((Vertex*)0)->normal == Vertex::kNormalOffset &&
                  (int)&((Vertex*)0)->boneIndices == Vertex::kBoneIndicesOffset &&
                  (int)&((Vertex*)0)->texCoords == Vertex::kTexCoordsOffset)
                     ? 1
                     : -1];
};
typedef char SkinSourceOffsetsValid[(sizeof(SkinSourceLayoutCheck<2>) == 1 && sizeof(SkinSourceLayoutCheck<3>) == 1 &&
                                     sizeof(SkinSourceLayoutCheck<4>) == 1)
                                        ? 1
                                        : -1];
typedef SkinOutputVertex<3> SkinOutputLayout;
typedef char SkinOutputOffsetsValid[((int)&((SkinOutputLayout*)0)->normal == SkinOutputLayout::kNormalOffset &&
                                     (int)&((SkinOutputLayout*)0)->texCoords == SkinOutputLayout::kTexCoordsOffset)
                                        ? 1
                                        : -1];
enum {
    kUnusedBoneIndex = 0x20,
    kMatrixStrideShift = 6
};
static const unsigned int kGpuFifoAddress = 0xCC008000;

// Byte cursors in several loops deliberately use these layout offsets: direct
// typed UV indexing changes CodeWarrior's register allocation or loop induction.

// Explicit workspaces retain the original stack slot order and palette address ABI.
struct SkinBlendFrame {
    union {
        unsigned int paletteAddress;
        void* palette;
    };
    float weight;
    float accumulatedWeight;
};
struct SkinSubmitFrame {
    void* vertices;
    unsigned int vertexCount;
    union {
        unsigned int paletteAddress;
        void* palette;
    };
    float normal[3];
    float position[3];
    float matrix[16];
};
struct SkinSubmit3Tex1Frame {
    void* vertices;
    unsigned int vertexCount;
    unsigned int paletteAddress;
    char reserved[8];
    float normal[3];
    float position[3];
    float matrix[16];
    char tailPadding[4];
};
struct SkinSubmit4Tex1Frame : SkinSubmitFrame {
    char tailPadding[12];
};
struct SkinTransform4Frame {
    void* outputVertices;
    void* sourceVertices;
    unsigned int vertexCount;
    void* palette;
    float matrix[16];
};

extern "C" {

void* Mtx44_BlendIndexed3(void*, unsigned int, void*, void*);
void Mtx34_TransformPointAndDirection(void*, void*, void*, void*, void*);
void Skin_TransformPositionAndNormalProjective(void*, void*, void*, void*, void*);
void Render_WriteFifo3f(float, float, float);
void Render_WriteTexCoord2f(float, float);
void Mtx44_BlendIndexed2(void*, void*, void*, void*);
void* Mtx44_BlendIndexed4(void*, void*, void*, void*);

typedef __vec2x32float__ Vec2;

#pragma push
#pragma optimization_level 2
// Blend up to three indexed matrices using eight paired-single accumulators.
void* Mtx44_BlendIndexed3(register void* outputMatrix, unsigned int matrixPalette, register void* weights,
                          register void* indices) {
    __alloca(0);
    SkinBlendFrame frame;
    frame.paletteAddress = matrixPalette;
    frame.accumulatedWeight = 0.0f;
    register unsigned char* boneIndices = (unsigned char*)indices;
    register float* explicitWeights = (float*)weights;
    register unsigned int influenceIndex;
    register float zero = 0.0f;
    register Vec2 sumPair0 = zero, sumPair1 = sumPair0, sumPair2 = sumPair0, sumPair3 = sumPair0;
    register Vec2 sumPair4 = sumPair0, sumPair5 = sumPair0, sumPair6 = sumPair0, sumPair7 = sumPair0;
    influenceIndex = 0;
    while (influenceIndex < 3) {
        register unsigned int boneIndex = boneIndices[influenceIndex];
        if (boneIndex == kUnusedBoneIndex)
            break;
        if (influenceIndex == 2)
            frame.weight = 1.0f - frame.accumulatedWeight;
        else
            frame.weight = explicitWeights[influenceIndex];
        register Vec2* matrixPairs = (Vec2*)((char*)frame.paletteAddress + (boneIndex << kMatrixStrideShift));
        register Vec2 weightPair = frame.weight;
        frame.accumulatedWeight += frame.weight;
        sumPair0 = sumPair0 + matrixPairs[0] * weightPair;
        sumPair1 = sumPair1 + matrixPairs[1] * weightPair;
        sumPair2 = sumPair2 + matrixPairs[2] * weightPair;
        sumPair3 = sumPair3 + matrixPairs[3] * weightPair;
        sumPair4 = sumPair4 + matrixPairs[4] * weightPair;
        sumPair5 = sumPair5 + matrixPairs[5] * weightPair;
        sumPair6 = sumPair6 + matrixPairs[6] * weightPair;
        sumPair7 = sumPair7 + matrixPairs[7] * weightPair;
        influenceIndex++;
    }
    register Vec2* outputPairs = (Vec2*)outputMatrix;
    outputPairs[0] = sumPair0;
    outputPairs[1] = sumPair1;
    outputPairs[2] = sumPair2;
    outputPairs[3] = sumPair3;
    outputPairs[4] = sumPair4;
    outputPairs[5] = sumPair5;
    outputPairs[6] = sumPair6;
    outputPairs[7] = sumPair7;
    return outputMatrix;
}
#pragma pop

#pragma push
#pragma optimization_level 2
// Blend up to four indexed matrices using eight paired-single accumulators.
void* Mtx44_BlendIndexed4(register void* outputMatrix, void* matrixPalette, register void* weights,
                          register void* indices) {
    __alloca(0);
    SkinBlendFrame frame;
    frame.palette = matrixPalette;
    frame.accumulatedWeight = 0.0f;
    register float* explicitWeights = (float*)weights;
    register unsigned char* boneIndices = (unsigned char*)indices;
    register unsigned int influenceIndex;
    register float zero = 0.0f;
    register Vec2 sumPair0 = zero, sumPair1 = sumPair0, sumPair2 = sumPair0, sumPair3 = sumPair0;
    register Vec2 sumPair4 = sumPair0, sumPair5 = sumPair0, sumPair6 = sumPair0, sumPair7 = sumPair0;
    influenceIndex = 0;
    while (influenceIndex < 4) {
        register unsigned int boneIndex = boneIndices[influenceIndex];
        if (boneIndex == kUnusedBoneIndex)
            break;
        if (influenceIndex == 3)
            frame.weight = 1.0f - frame.accumulatedWeight;
        else
            frame.weight = explicitWeights[influenceIndex];
        register Vec2* matrixPairs = (Vec2*)((char*)frame.palette + (boneIndex << kMatrixStrideShift));
        register Vec2 weightPair = frame.weight;
        frame.accumulatedWeight += frame.weight;
        sumPair0 = sumPair0 + matrixPairs[0] * weightPair;
        sumPair1 = sumPair1 + matrixPairs[1] * weightPair;
        sumPair2 = sumPair2 + matrixPairs[2] * weightPair;
        sumPair3 = sumPair3 + matrixPairs[3] * weightPair;
        sumPair4 = sumPair4 + matrixPairs[4] * weightPair;
        sumPair5 = sumPair5 + matrixPairs[5] * weightPair;
        sumPair6 = sumPair6 + matrixPairs[6] * weightPair;
        sumPair7 = sumPair7 + matrixPairs[7] * weightPair;
        influenceIndex++;
    }
    Vec2* outputPairs = (Vec2*)outputMatrix;
    outputPairs[0] = sumPair0;
    outputPairs[1] = sumPair1;
    outputPairs[2] = sumPair2;
    outputPairs[3] = sumPair3;
    outputPairs[4] = sumPair4;
    outputPairs[5] = sumPair5;
    outputPairs[6] = sumPair6;
    outputPairs[7] = sumPair7;
    return outputMatrix;
}
#pragma pop

// Blend 3 bone matrices and submit position, normal and 1 UV set(s) to the render FIFO.
void Skin_Submit3WeightPositionNormalTex1(void* vertices, unsigned int vertexCount, unsigned int matrixPalette) {
    __alloca(0);
    SkinSubmit3Tex1Frame frame;
    frame.vertices = vertices;
    frame.vertexCount = vertexCount;
    frame.paletteAddress = matrixPalette;
    unsigned int vertexIndex = 0;
    SkinSourceVertex<3, 1>* sourceVertex = (SkinSourceVertex<3, 1>*)vertices;
    for (; vertexIndex < frame.vertexCount; vertexIndex++) {
        Mtx44_BlendIndexed3(frame.matrix, frame.paletteAddress, sourceVertex->weights, sourceVertex->boneIndices);
        Skin_TransformPositionAndNormalProjective(frame.position, frame.normal, sourceVertex, sourceVertex->normal,
                                                  frame.matrix);
        Render_WriteFifo3f(frame.position[0], frame.position[1], frame.position[2]);
        Render_WriteFifo3f(frame.normal[0], frame.normal[1], frame.normal[2]);
        Render_WriteTexCoord2f(sourceVertex->texCoords[0][0], sourceVertex->texCoords[0][1]);
        sourceVertex += 1;
    }
}

// Blend 3 bone matrices and write transformed position/normal plus 1 UV set(s) per vertex.
void Skin_Transform3WeightPositionNormalTex1(void* outputVertices, void* sourceVertices, unsigned int vertexCount,
                                             unsigned int unusedFormat, unsigned int matrixPalette) {
    __alloca(0);
    unsigned int vertexIndex = 0;
    SkinSourceVertex<3, 1>* sourceVertex = (SkinSourceVertex<3, 1>*)sourceVertices;
    SkinOutputVertex<1>* outputVertex = (SkinOutputVertex<1>*)outputVertices;
    for (; vertexIndex < vertexCount; vertexIndex++) {
        float blendedMatrix[16];
        Mtx44_BlendIndexed3(blendedMatrix, matrixPalette, sourceVertex->weights, sourceVertex->boneIndices);
        Mtx34_TransformPointAndDirection(outputVertex, outputVertex->normal, sourceVertex, sourceVertex->normal,
                                         blendedMatrix);
        outputVertex->texCoords[0][0] = sourceVertex->texCoords[0][0];
        outputVertex->texCoords[0][1] = sourceVertex->texCoords[0][1];
        sourceVertex += 1;
        outputVertex += 1;
    }
}

#pragma push
#pragma optimization_level 0
// Blend 2 bone matrices and submit position, normal and 2 UV set(s) to the render FIFO.
void Skin_Submit2WeightPositionNormalTex2(void* vertices, unsigned int vertexCount, void* matrixPalette) {
    __alloca(0);
    SkinSubmitFrame frame;
    frame.vertices = vertices;
    unsigned int vertexIndex = 0;
    int texCoordIndex;
    SkinSourceVertex<2, 2>* sourceVertex = (SkinSourceVertex<2, 2>*)vertices;
    frame.vertexCount = vertexCount;
    frame.palette = matrixPalette;
    for (; vertexIndex < frame.vertexCount; vertexIndex++) {
        Mtx44_BlendIndexed2(frame.matrix, frame.palette, sourceVertex->weights, sourceVertex->boneIndices);
        Skin_TransformPositionAndNormalProjective(frame.position, frame.normal, sourceVertex, sourceVertex->normal,
                                                  frame.matrix);
        Render_WriteFifo3f(frame.position[0], frame.position[1], frame.position[2]);
        Render_WriteFifo3f(frame.normal[0], frame.normal[1], frame.normal[2]);
        for (texCoordIndex = 0; texCoordIndex < 2; texCoordIndex++) {
            Render_WriteTexCoord2f(sourceVertex->texCoords[texCoordIndex][0],
                                   sourceVertex->texCoords[texCoordIndex][1]);
        }
        sourceVertex += 1;
    }
}
#pragma pop

#pragma push
#pragma optimization_level 0
// Blend 2 bone matrices and write transformed position/normal plus 2 UV set(s) per vertex.
void Skin_Transform2WeightPositionNormalTex2(void* outputVertices, void* sourceVertices, unsigned int vertexCount,
                                             unsigned int unusedFormat, void* matrixPalette) {
    float blendedMatrix[16];
    unsigned int vertexIndex = 0;
    char* outputVertex = (char*)outputVertices;
    int texCoordIndex;
    char* sourceVertex = (char*)sourceVertices;
    __alloca(0);
    for (; vertexIndex < vertexCount; vertexIndex++) {
        Mtx44_BlendIndexed2(blendedMatrix, matrixPalette, sourceVertex + SkinSourceVertex<2, 2>::kWeightsOffset,
                            sourceVertex + SkinSourceVertex<2, 2>::kBoneIndicesOffset);
        Mtx34_TransformPointAndDirection(outputVertex, outputVertex + SkinOutputVertex<2>::kNormalOffset, sourceVertex,
                                         sourceVertex + SkinSourceVertex<2, 2>::kNormalOffset, blendedMatrix);
        for (texCoordIndex = 0; texCoordIndex < 2; texCoordIndex++) {
            *(float*)(outputVertex + texCoordIndex * 8 + SkinOutputVertex<2>::kTexCoordsOffset) =
                *(float*)(sourceVertex + texCoordIndex * 8 + SkinSourceVertex<2, 2>::kTexCoordsOffset);
            *(float*)(outputVertex + texCoordIndex * 8 + SkinOutputVertex<2>::kTexCoordsOffset + 4) =
                *(float*)(sourceVertex + texCoordIndex * 8 + SkinSourceVertex<2, 2>::kTexCoordsOffset + 4);
        }
        sourceVertex += sizeof(SkinSourceVertex<2, 2>);
        outputVertex += sizeof(SkinOutputVertex<2>);
    }
}
#pragma pop

// Blend 2 bone matrices and write transformed position/normal plus 1 UV set(s) per vertex.
void Skin_Transform2WeightPositionNormalTex1(void* outputVertices, void* sourceVertices, unsigned int vertexCount,
                                             unsigned int unusedFormat, void* matrixPalette) {
    __alloca(0);
    unsigned int vertexIndex = 0;
    SkinOutputVertex<1>* outputVertex = (SkinOutputVertex<1>*)outputVertices;
    SkinSourceVertex<2, 1>* sourceVertex = (SkinSourceVertex<2, 1>*)sourceVertices;
    for (; vertexIndex < vertexCount; vertexIndex++) {
        if (sourceVertex->boneIndices[1] == kUnusedBoneIndex) {
            unsigned char index = sourceVertex->boneIndices[0];
            Mtx34_TransformPointAndDirection(outputVertex, outputVertex->normal, sourceVertex, sourceVertex->normal,
                                             (char*)matrixPalette + (index << kMatrixStrideShift));
        } else {
            float blendedMatrix[16];
            Mtx44_BlendIndexed2(blendedMatrix, matrixPalette, sourceVertex->weights, sourceVertex->boneIndices);
            Mtx34_TransformPointAndDirection(outputVertex, outputVertex->normal, sourceVertex, sourceVertex->normal,
                                             blendedMatrix);
        }
        outputVertex->texCoords[0][0] = sourceVertex->texCoords[0][0];
        outputVertex->texCoords[0][1] = sourceVertex->texCoords[0][1];
        sourceVertex += 1;
        outputVertex += 1;
    }
}

// Reserved skinning entry point; this implementation performs no work.
void Skin_ReservedNoOp_80431370() {}
// Reserved skinning entry point; this implementation performs no work.
void Skin_ReservedNoOp_80431534() {}

// Blend 2 bone matrices and submit position, normal and 3 UV set(s) to the render FIFO.
void Skin_Submit2WeightPositionNormalTex3(void* vertices, unsigned int vertexCount, void* matrixPalette) {
    __alloca(0);
    unsigned int vertexLimit = vertexCount;
    SkinSourceVertex<2, 3>* sourceVertex = (SkinSourceVertex<2, 3>*)vertices;
    int texCoordIndex;
    unsigned int vertexIndex = 0;
    for (; vertexIndex < vertexLimit; vertexIndex++) {
        float blendedMatrix[16];
        float position[3];
        float normal[3];
        Mtx44_BlendIndexed2(blendedMatrix, matrixPalette, sourceVertex->weights, sourceVertex->boneIndices);
        Skin_TransformPositionAndNormalProjective(position, normal, sourceVertex, sourceVertex->normal, blendedMatrix);
        Render_WriteFifo3f(position[0], position[1], position[2]);
        Render_WriteFifo3f(normal[0], normal[1], normal[2]);
        for (texCoordIndex = 0; texCoordIndex < 3; texCoordIndex++) {
            Render_WriteTexCoord2f(
                *(float*)((char*)sourceVertex + texCoordIndex * 8 + SkinSourceVertex<2, 3>::kTexCoordsOffset),
                *(float*)((char*)sourceVertex + texCoordIndex * 8 + SkinSourceVertex<2, 3>::kTexCoordsOffset + 4));
        }
        sourceVertex += 1;
    }
}

#pragma push
#pragma optimization_level 0
// Blend 2 bone matrices and write transformed position/normal plus 3 UV set(s) per vertex.
void Skin_Transform2WeightPositionNormalTex3(void* outputVertices, void* sourceVertices, unsigned int vertexCount,
                                             unsigned int unusedFormat, void* matrixPalette) {
    float blendedMatrix[16];
    unsigned int vertexIndex = 0;
    char* outputVertex = (char*)outputVertices;
    int texCoordIndex;
    char* sourceVertex = (char*)sourceVertices;
    (void)outputVertex;
    __alloca(0);
    for (; vertexIndex < vertexCount; vertexIndex++) {
        Mtx44_BlendIndexed2(blendedMatrix, matrixPalette, sourceVertex + SkinSourceVertex<2, 3>::kWeightsOffset,
                            sourceVertex + SkinSourceVertex<2, 3>::kBoneIndicesOffset);
        Mtx34_TransformPointAndDirection(outputVertex, outputVertex + SkinOutputVertex<3>::kNormalOffset, sourceVertex,
                                         sourceVertex + SkinSourceVertex<2, 3>::kNormalOffset, blendedMatrix);
        for (texCoordIndex = 0; texCoordIndex < 3; texCoordIndex++) {
            *(float*)(outputVertex + texCoordIndex * 8 + SkinOutputVertex<3>::kTexCoordsOffset) =
                *(float*)(sourceVertex + texCoordIndex * 8 + SkinSourceVertex<2, 3>::kTexCoordsOffset);
            *(float*)(outputVertex + texCoordIndex * 8 + SkinOutputVertex<3>::kTexCoordsOffset + 4) =
                *(float*)(sourceVertex + texCoordIndex * 8 + SkinSourceVertex<2, 3>::kTexCoordsOffset + 4);
        }
        sourceVertex += sizeof(SkinSourceVertex<2, 3>);
        outputVertex += sizeof(SkinOutputVertex<3>);
    }
}

#pragma pop
#pragma push
#pragma optimization_level 0
// Blend 3 bone matrices and write transformed position/normal plus 2 UV set(s) per vertex.
void Skin_Transform3WeightPositionNormalTex2(void* outputVertices, void* sourceVertices, unsigned int vertexCount,
                                             unsigned int unusedFormat, void* matrixPalette) {
    __alloca(0);
    float frame[16];
    unsigned int vertexIndex = 0;
    char* outputVertex = (char*)outputVertices;
    int texCoordIndex;
    char* sourceVertex = (char*)sourceVertices;
    for (; vertexIndex < vertexCount; vertexIndex++) {
        Mtx44_BlendIndexed3(frame, (unsigned int)matrixPalette, sourceVertex + SkinSourceVertex<3, 2>::kWeightsOffset,
                            sourceVertex + SkinSourceVertex<3, 2>::kBoneIndicesOffset);
        Mtx34_TransformPointAndDirection(outputVertex, outputVertex + SkinOutputVertex<2>::kNormalOffset, sourceVertex,
                                         sourceVertex + SkinSourceVertex<3, 2>::kNormalOffset, frame);
        for (texCoordIndex = 0; texCoordIndex < 2; texCoordIndex++) {
            *(float*)(outputVertex + texCoordIndex * 8 + SkinOutputVertex<2>::kTexCoordsOffset) =
                *(float*)(sourceVertex + texCoordIndex * 8 + SkinSourceVertex<3, 2>::kTexCoordsOffset);
            *(float*)(outputVertex + texCoordIndex * 8 + SkinOutputVertex<2>::kTexCoordsOffset + 4) =
                *(float*)(sourceVertex + texCoordIndex * 8 + SkinSourceVertex<3, 2>::kTexCoordsOffset + 4);
        }
        sourceVertex += sizeof(SkinSourceVertex<3, 2>);
        outputVertex += sizeof(SkinOutputVertex<2>);
    }
}
#pragma pop

// Blend 3 bone matrices and submit position, normal and 3 UV set(s) to the render FIFO.
void Skin_Submit3WeightPositionNormalTex3(void* vertices, unsigned int vertexCount, unsigned int matrixPalette) {
    SkinSubmitFrame frame;
    frame.vertices = vertices;
    unsigned int vertexIndex = 0;
    int texCoordIndex;
    SkinSourceVertex<3, 3>* sourceVertex = (SkinSourceVertex<3, 3>*)vertices;
    frame.vertexCount = vertexCount;
    frame.paletteAddress = matrixPalette;
    __alloca(0);
    for (; vertexIndex < frame.vertexCount; vertexIndex++) {
        Mtx44_BlendIndexed3(frame.matrix, frame.paletteAddress, sourceVertex->weights, sourceVertex->boneIndices);
        Skin_TransformPositionAndNormalProjective(frame.position, frame.normal, sourceVertex, sourceVertex->normal,
                                                  frame.matrix);
        Render_WriteFifo3f(frame.position[0], frame.position[1], frame.position[2]);
        Render_WriteFifo3f(frame.normal[0], frame.normal[1], frame.normal[2]);
        for (texCoordIndex = 0; texCoordIndex < 3; texCoordIndex++) {
            Render_WriteTexCoord2f(
                *(float*)((char*)sourceVertex + texCoordIndex * 8 + SkinSourceVertex<3, 3>::kTexCoordsOffset),
                *(float*)((char*)sourceVertex + texCoordIndex * 8 + SkinSourceVertex<3, 3>::kTexCoordsOffset + 4));
        }
        sourceVertex += 1;
    }
}

#pragma push
#pragma optimization_level 0
// Blend 3 bone matrices and write transformed position/normal plus 3 UV set(s) per vertex.
void Skin_Transform3WeightPositionNormalTex3(void* outputVertices, void* sourceVertices, unsigned int vertexCount,
                                             unsigned int unusedFormat, void* matrixPalette) {
    float blendedMatrix[16];
    unsigned int vertexIndex = 0;
    char* outputVertex = (char*)outputVertices;
    int texCoordIndex;
    char* sourceVertex = (char*)sourceVertices;
    __alloca(0);
    for (; vertexIndex < vertexCount; vertexIndex++) {
        Mtx44_BlendIndexed3(blendedMatrix, (unsigned int)matrixPalette,
                            sourceVertex + SkinSourceVertex<3, 3>::kWeightsOffset,
                            sourceVertex + SkinSourceVertex<3, 3>::kBoneIndicesOffset);
        Mtx34_TransformPointAndDirection(outputVertex, outputVertex + SkinOutputVertex<3>::kNormalOffset, sourceVertex,
                                         sourceVertex + SkinSourceVertex<3, 3>::kNormalOffset, blendedMatrix);
        for (texCoordIndex = 0; texCoordIndex < 3; texCoordIndex++) {
            *(float*)(outputVertex + texCoordIndex * 8 + SkinOutputVertex<3>::kTexCoordsOffset) =
                *(float*)(sourceVertex + texCoordIndex * 8 + SkinSourceVertex<3, 3>::kTexCoordsOffset);
            *(float*)(outputVertex + texCoordIndex * 8 + SkinOutputVertex<3>::kTexCoordsOffset + 4) =
                *(float*)(sourceVertex + texCoordIndex * 8 + SkinSourceVertex<3, 3>::kTexCoordsOffset + 4);
        }
        sourceVertex += sizeof(SkinSourceVertex<3, 3>);
        outputVertex += sizeof(SkinOutputVertex<3>);
    }
}
#pragma pop

// Blend 3 bone matrices and submit position, normal and 2 UV set(s) to the render FIFO.
void Skin_Submit3WeightPositionNormalTex2(void* vertices, unsigned int vertexCount, unsigned int matrixPalette) {
    SkinSubmitFrame frame;
    frame.vertices = vertices;
    unsigned int vertexIndex = 0;
    int texCoordIndex;
    SkinSourceVertex<3, 2>* sourceVertex = (SkinSourceVertex<3, 2>*)vertices;
    frame.vertexCount = vertexCount;
    frame.paletteAddress = matrixPalette;
    __alloca(0);
    for (; vertexIndex < frame.vertexCount; vertexIndex++) {
        Mtx44_BlendIndexed3(frame.matrix, frame.paletteAddress, sourceVertex->weights, sourceVertex->boneIndices);
        Skin_TransformPositionAndNormalProjective(frame.position, frame.normal, sourceVertex, sourceVertex->normal,
                                                  frame.matrix);
        Render_WriteFifo3f(frame.position[0], frame.position[1], frame.position[2]);
        Render_WriteFifo3f(frame.normal[0], frame.normal[1], frame.normal[2]);
        for (texCoordIndex = 0; texCoordIndex < 2; texCoordIndex++) {
            Render_WriteTexCoord2f(
                *(float*)((char*)sourceVertex + texCoordIndex * 8 + SkinSourceVertex<3, 2>::kTexCoordsOffset),
                *(float*)((char*)sourceVertex + texCoordIndex * 8 + SkinSourceVertex<3, 2>::kTexCoordsOffset + 4));
        }
        sourceVertex += 1;
    }
}

#pragma push
#pragma optimization_level 0
// Write each 16-bit attribute three times to the GPU FIFO; retain volatile loads/stores.
void Skin_SubmitTriplicatedU16Attribute(unsigned short* data, volatile unsigned int count) {
    __alloca(0);
    unsigned int i = 0;
    for (; i < count; i++) {
        *(volatile unsigned short*)kGpuFifoAddress = data[0];
        *(volatile unsigned short*)kGpuFifoAddress = data[0];
        *(volatile unsigned short*)kGpuFifoAddress = data[0];
        data++;
    }
}
#pragma pop

// Reserved skinning entry point; this implementation performs no work.
void Skin_ReservedNoOp_80431A00() {}
// Reserved skinning entry point; this implementation performs no work.
void Skin_ReservedNoOp_80431BC4() {}
// Reserved skinning entry point; this implementation performs no work.
void Skin_ReservedNoOp_80431EC4() {}
// Reserved skinning entry point; this implementation performs no work.
void Skin_ReservedNoOp_80432088() {}

#pragma push
#pragma optimization_level 0
// Blend 4 bone matrices and write transformed position/normal plus 2 UV set(s) per vertex.
void Skin_Transform4WeightPositionNormalTex2(void* outputVertices, void* sourceVertices, unsigned int vertexCount,
                                             unsigned int unusedFormat, void* matrixPalette) {
    SkinTransform4Frame frame;
    frame.outputVertices = outputVertices;
    unsigned int vertexIndex = 0;
    int texCoordIndex;
    char* outputVertex = (char*)outputVertices;
    char* sourceVertex = (char*)sourceVertices;
    frame.sourceVertices = sourceVertices;
    frame.vertexCount = vertexCount;
    frame.palette = matrixPalette;
    for (; vertexIndex < frame.vertexCount; vertexIndex++) {
        Mtx44_BlendIndexed4(frame.matrix, frame.palette, sourceVertex + SkinSourceVertex<4, 2>::kWeightsOffset,
                            sourceVertex + SkinSourceVertex<4, 2>::kBoneIndicesOffset);
        Mtx34_TransformPointAndDirection(outputVertex, outputVertex + SkinOutputVertex<2>::kNormalOffset, sourceVertex,
                                         sourceVertex + SkinSourceVertex<4, 2>::kNormalOffset, frame.matrix);
        for (texCoordIndex = 0; texCoordIndex < 2; texCoordIndex++) {
            *(float*)(outputVertex + texCoordIndex * 8 + SkinOutputVertex<2>::kTexCoordsOffset) =
                *(float*)(sourceVertex + texCoordIndex * 8 + SkinSourceVertex<4, 2>::kTexCoordsOffset);
            *(float*)(outputVertex + texCoordIndex * 8 + SkinOutputVertex<2>::kTexCoordsOffset + 4) =
                *(float*)(sourceVertex + texCoordIndex * 8 + SkinSourceVertex<4, 2>::kTexCoordsOffset + 4);
        }
        sourceVertex += sizeof(SkinSourceVertex<4, 2>);
        outputVertex += sizeof(SkinOutputVertex<2>);
    }
}
#pragma pop

#pragma push
#pragma optimization_level 0
// Blend 4 bone matrices and submit position, normal and 3 UV set(s) to the render FIFO.
void Skin_Submit4WeightPositionNormalTex3(void* vertices, unsigned int vertexCount, void* matrixPalette) {
    __alloca(0);
    SkinSubmitFrame frame;
    frame.vertices = vertices;
    register unsigned int vertexIndex = 0;
    register int texCoordIndex;
    register SkinSourceVertex<4, 3>* sourceVertex = (SkinSourceVertex<4, 3>*)vertices;
    frame.vertexCount = vertexCount;
    frame.palette = matrixPalette;
    for (; vertexIndex < frame.vertexCount; vertexIndex++) {
        Mtx44_BlendIndexed4(frame.matrix, frame.palette, sourceVertex->weights, sourceVertex->boneIndices);
        Skin_TransformPositionAndNormalProjective(frame.position, frame.normal, sourceVertex, sourceVertex->normal,
                                                  frame.matrix);
        Render_WriteFifo3f(frame.position[0], frame.position[1], frame.position[2]);
        Render_WriteFifo3f(frame.normal[0], frame.normal[1], frame.normal[2]);
        for (texCoordIndex = 0; texCoordIndex < 3; texCoordIndex++) {
            Render_WriteTexCoord2f(
                ((float*)((char*)sourceVertex + SkinSourceVertex<4, 3>::kTexCoordsOffset))[texCoordIndex * 2],
                ((float*)((char*)sourceVertex + SkinSourceVertex<4, 3>::kTexCoordsOffset))[texCoordIndex * 2 + 1]);
        }
        sourceVertex += 1;
    }
}
#pragma pop

#pragma push
#pragma optimization_level 0
// Blend 4 bone matrices and write transformed position/normal plus 3 UV set(s) per vertex.
void Skin_Transform4WeightPositionNormalTex3(void* outputVertices, void* sourceVertices, unsigned int vertexCount,
                                             unsigned int unusedFormat, void* matrixPalette) {
    float blendedMatrix[16];
    unsigned int vertexIndex = 0;
    char* outputVertex = (char*)outputVertices;
    int texCoordIndex;
    char* sourceVertex = (char*)sourceVertices;
    __alloca(0);
    for (; vertexIndex < vertexCount; vertexIndex++) {
        Mtx44_BlendIndexed4(blendedMatrix, matrixPalette, sourceVertex + SkinSourceVertex<4, 3>::kWeightsOffset,
                            sourceVertex + SkinSourceVertex<4, 3>::kBoneIndicesOffset);
        Mtx34_TransformPointAndDirection(outputVertex, outputVertex + SkinOutputVertex<3>::kNormalOffset, sourceVertex,
                                         sourceVertex + SkinSourceVertex<4, 3>::kNormalOffset, blendedMatrix);
        for (texCoordIndex = 0; texCoordIndex < 3; texCoordIndex++) {
            *(float*)(outputVertex + texCoordIndex * 8 + SkinOutputVertex<3>::kTexCoordsOffset) =
                *(float*)(sourceVertex + texCoordIndex * 8 + SkinSourceVertex<4, 3>::kTexCoordsOffset);
            *(float*)(outputVertex + texCoordIndex * 8 + SkinOutputVertex<3>::kTexCoordsOffset + 4) =
                *(float*)(sourceVertex + texCoordIndex * 8 + SkinSourceVertex<4, 3>::kTexCoordsOffset + 4);
        }
        sourceVertex += sizeof(SkinSourceVertex<4, 3>);
        outputVertex += sizeof(SkinOutputVertex<3>);
    }
}
#pragma pop

// Reserved skinning entry point; this implementation performs no work.
void Skin_ReservedNoOp_8043224C() {}

// Blend 4 bone matrices and submit position, normal and 1 UV set(s) to the render FIFO.
void Skin_Submit4WeightPositionNormalTex1(void* vertices, unsigned int vertexCount, void* matrixPalette) {
    SkinSubmit4Tex1Frame frame;
    frame.vertices = vertices;
    unsigned int vertexIndex = 0;
    SkinSourceVertex<4, 1>* sourceVertex = (SkinSourceVertex<4, 1>*)vertices;
    frame.vertexCount = vertexCount;
    frame.palette = matrixPalette;
    for (; vertexIndex < frame.vertexCount; vertexIndex++) {
        Mtx44_BlendIndexed4(frame.matrix, frame.palette, sourceVertex->weights, sourceVertex->boneIndices);
        Skin_TransformPositionAndNormalProjective(frame.position, frame.normal, sourceVertex, sourceVertex->normal,
                                                  frame.matrix);
        Render_WriteFifo3f(frame.position[0], frame.position[1], frame.position[2]);
        Render_WriteFifo3f(frame.normal[0], frame.normal[1], frame.normal[2]);
        for (int texCoordIndex = 0; texCoordIndex < 1; texCoordIndex++) {
            Render_WriteTexCoord2f(sourceVertex->texCoords[texCoordIndex][0],
                                   sourceVertex->texCoords[texCoordIndex][1]);
        }
        sourceVertex += 1;
    }
}

#pragma push
#pragma optimization_level 0
// Blend 4 bone matrices and write transformed position/normal plus 1 UV set(s) per vertex.
void Skin_Transform4WeightPositionNormalTex1(void* outputVertices, void* sourceVertices, unsigned int vertexCount,
                                             unsigned int unusedFormat, void* something) {
    __alloca(0);
    SkinTransform4Frame frame;
    unsigned int vertexIndex = 0;
    int texCoordIndex;
    char* outputVertex = (char*)(frame.outputVertices = outputVertices);
    char* sourceVertex = (char*)(frame.sourceVertices = sourceVertices);
    frame.vertexCount = vertexCount;
    frame.palette = something;
    for (; vertexIndex < frame.vertexCount; vertexIndex++) {
        Mtx44_BlendIndexed4(frame.matrix, frame.palette, sourceVertex + SkinSourceVertex<4, 1>::kWeightsOffset,
                            sourceVertex + SkinSourceVertex<4, 1>::kBoneIndicesOffset);
        Mtx34_TransformPointAndDirection(outputVertex, outputVertex + SkinOutputVertex<1>::kNormalOffset, sourceVertex,
                                         sourceVertex + SkinSourceVertex<4, 1>::kNormalOffset, frame.matrix);
        for (texCoordIndex = 0; texCoordIndex < 1; texCoordIndex++) {
            *(float*)(outputVertex + texCoordIndex * 8 + SkinOutputVertex<1>::kTexCoordsOffset) =
                *(float*)(sourceVertex + texCoordIndex * 8 + SkinSourceVertex<4, 1>::kTexCoordsOffset);
            *(float*)(outputVertex + texCoordIndex * 8 + SkinOutputVertex<1>::kTexCoordsOffset + 4) =
                *(float*)(sourceVertex + texCoordIndex * 8 + SkinSourceVertex<4, 1>::kTexCoordsOffset + 4);
        }
        sourceVertex += sizeof(SkinSourceVertex<4, 1>);
        outputVertex += sizeof(SkinOutputVertex<1>);
    }
}
#pragma pop

#pragma push
#pragma optimization_level 0
// Blend 4 bone matrices and submit position, normal and 2 UV set(s) to the render FIFO.
void Skin_Submit4WeightPositionNormalTex2(void* vertices, unsigned int vertexCount, void* matrixPalette) {
    __alloca(0);
    SkinSubmitFrame frame;
    frame.vertices = vertices;
    register unsigned int vertexIndex = 0;
    register int texCoordIndex;
    register SkinSourceVertex<4, 2>* sourceVertex = (SkinSourceVertex<4, 2>*)vertices;
    frame.vertexCount = vertexCount;
    frame.palette = matrixPalette;
    for (; vertexIndex < frame.vertexCount; vertexIndex++) {
        Mtx44_BlendIndexed4(frame.matrix, frame.palette, sourceVertex->weights, sourceVertex->boneIndices);
        Skin_TransformPositionAndNormalProjective(frame.position, frame.normal, sourceVertex, sourceVertex->normal,
                                                  frame.matrix);
        Render_WriteFifo3f(frame.position[0], frame.position[1], frame.position[2]);
        Render_WriteFifo3f(frame.normal[0], frame.normal[1], frame.normal[2]);
        for (texCoordIndex = 0; texCoordIndex < 2; texCoordIndex++) {
            Render_WriteTexCoord2f(sourceVertex->texCoords[texCoordIndex][0],
                                   sourceVertex->texCoords[texCoordIndex][1]);
        }
        sourceVertex += 1;
    }
}
#pragma pop

} // extern "C"
