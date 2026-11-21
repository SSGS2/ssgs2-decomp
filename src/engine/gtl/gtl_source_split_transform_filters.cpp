#pragma push
#include "classes/gtl_CSourceFilter.h"
#define Asgtl_CSourceFilter(p) ((gtl_CSourceFilter*)(p))
#include "classes/gtl_CSplitFilter.h"
#define Asgtl_CSplitFilter(p) ((gtl_CSplitFilter*)(p))
#include "classes/gtl_CTransformFilter.h"
#define Asgtl_CTransformFilter(p) ((gtl_CTransformFilter*)(p))
// Translation unit 0x803F7B14-0x803F8E58 (30 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

// Shared filter state/result codes; Process may succeed while waiting for input.
enum FilterState {
    kFilterReset = 0, kFilterRunning = 1, kFilterWaitingForInput = 2,
    kFilterStopped = 3, kFilterOutputReady = 4, kFilterFormatsRejected = 6,
    kFilterProcessFailed = 7
};
enum { kSourceReadBufferBytes = 0x40000, kSourceReadAlignment = 4, kStoredValueUnitShift = 7 };

struct SourceReadLatch {
	char baseStorage[0x18];
	unsigned char pendingRead;
};

struct StateValue {
	char baseStorage[0x10];
	unsigned int value;
};

struct SourceReadCursor {
	char baseStorage[0x24];
	unsigned int readOffset;
};

struct SourceResetState {
	char baseStorage[0x18];
	unsigned char pendingRead;
	char alignmentPadding[7];
	void* source;
	unsigned int readOffset;
};

struct SourceInputState {
	char baseStorage[0x10];
	void* input;
	unsigned int bufferAddress;
};

struct CallbackObject {
	char baseStorage[4];
	virtual void v0();
	virtual void v1();
	virtual void resetState();
	virtual void v3();
	virtual void resetInput();
};

typedef gtl_CTransformFilter TransformFilter;

typedef gtl_CSourceFilter SourceFilter;

struct RefObjectBase { unsigned int refs; };

struct TransformFilterVirtuals {
	char baseStorage[4];
	virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
	virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7();
	virtual int formatsMatch(void*, void*);
	virtual int allocateOutputBuffer(void*, void*);
	virtual int transformSample(void*, void*);
};

struct FilterVTable8808 {
	char baseStorage[4];
	virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
	virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7();
	virtual int formatsMatch(void*, void*);
	virtual int splitSample(void*, void*, void*);
};

typedef gtl_CSplitFilter SplitFilter;

struct RefObject : RefObjectBase {
	virtual void destroy(int deleting);
};

struct SourceReaderVirtuals {
	char baseStorage[4];
	virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
	virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7();
	virtual int validateSource(void*, int);
	virtual int readSample(void*, void*, unsigned int*);
	virtual int getBytesRead(unsigned int*);
};

void* gtl_Alloc(unsigned int);

// Clear queue counters and initialize an empty page-pointer vector.
void* GtlPagedQueue803F_Ctor(void* self);

// Accept a nonzero source-operation argument.
int gtl_CSourceFilter_vfunc_8(void* unused, int value);

// Return zero for a stored zero, otherwise scale the stored value minus one by 128.
unsigned int GtlPagedQueue803F_GetStorageEntryCapacity(StateValue* self);

// Write the current read offset when an output pointer is supplied.
int gtl_CSourceFilter_GetReadOffset(SourceReadCursor* self, unsigned int* out);

// Return the pending-read flag.
unsigned char gtl_CSourceFilter_IsReadPending(SourceReadLatch* self);

// Latch a pending read.
void gtl_CSourceFilter_MarkReadPending(SourceReadLatch* self);

// Clear the pending-read latch.
void gtl_CSourceFilter_ClearReadPending(SourceReadLatch* self);

extern void gtl_IFilter_SetState(void*, int);
extern void* gtl_IFilter_dtor_Base(void*, int);
extern void gtl_Free(void*);
extern void gtl_CMediaSample_CopyFormat(void*, void*);
extern int gtl_MediaFormat_Equals(void*, void*);
extern void gtl_IFilter_GetState(void*, int*);
extern void gtl_CMediaSample_GetData(void*, unsigned int*);
extern void gtl_CMediaSample_CopyFormat(void*, void*);
extern void gtl_CMediaSample_SetBuffer(void*, unsigned int, unsigned int);
extern void gtl_CMediaSample_GetSize(void*, unsigned int*);
extern int SamplePool_IsEmpty(void*);
extern int SamplePool_PeekAvailableSample(void*, void**);
extern void SamplePool_GetDimensions(void*, void*);
extern void SamplePool_CommitSample(void*, unsigned int);
extern int SamplePool_PeekAvailableSample(void*, void**);
extern void gtl_IFilter_Ctor(void*, void*, void*);
extern void gtl_CMediaSample_Ctor(void*, void*, void*, void*);
extern void* gtl_Alloc(unsigned int);
extern char __vt__Q23gtl16CTransformFilter[];
extern char __vt__Q23gtl13CSourceFilter[];
extern char __vt__Q23gtl12CSplitFilter[];

struct SourceData {
    unsigned int formatKind;
    unsigned int byteLength;
    char formatPayload[0x84];
    unsigned int firstReadOffset;
};
// Initialize the source filter and start reading at the format's initial byte offset.
void* gtl_CSourceFilter_Ctor(SourceFilter* self, const SourceData* source, void* callback);

// Release the old input reference and retain the replacement sample.
int gtl_CSplitFilter_SetInputSample(void* self, RefObject* value);

// Create a two-output filter and allocate both sample wrappers.
SplitFilter* gtl_CSplitFilter_Ctor(SplitFilter* self, void* input, void* arg2);

// Clear the pending read, restore the initial read offset, and reset the filter state.
int gtl_CSourceFilter_ResetReadState(SourceResetState* self);

// Store the input sample and cache its data pointer.
int gtl_CSourceFilter_StoreInputSample(SourceInputState* self, void* input);

// Start processing unless the source has already reached output-ready state.
int gtl_CSourceFilter_StartProcessing(void* self);

// Resume processing unless the source has already reached output-ready state.
int gtl_CSourceFilter_ResumeUnlessOutputReady(void* self);

// Dispatch the filter's input-reset and state-reset hooks in that order.
int gtl_CSourceFilter_ResetAndStart(CallbackObject* self);

// Set the split filter to running.
int gtl_CSplitFilter_SetRunning(void* self);

// Stop the split filter.
int gtl_CSplitFilter_Stop(void* self);

// Destroy the base filter and optionally free its allocation.
void* gtl_CSourceFilter_dtor(void* self, int deleting);

// Copy both sample format descriptions and compare their complete known fields.
int gtl_CSplitFilter_FormatsMatch(void* unused, void* left, void* right);

// Release the input sample and reset the filter.
int gtl_CSplitFilter_Reset(void* self);

// Add a reference to output zero or one; other selectors leave the output untouched.
int gtl_CSplitFilter_GetOutputSample(void* self, int which, unsigned int** out);

// Initialize the transform interface, empty input, output sample, and format-check latch.
TransformFilter* gtl_CTransformFilter_Ctor(TransformFilter* self, void* arg1, void* arg2);

// Expose output after a stop, otherwise set the transform filter running.
int gtl_CTransformFilter_ResumeOrExposeOutput(void* self);

// Validate input/output formats once, split the input, and publish success or the failure state.
int gtl_CSplitFilter_Process(FilterVTable8808* self);

// Release the transform input reference, destroy the base filter, and optionally free the object.
void* gtl_CTransformFilter_dtor(TransformFilter* self, int deleting);

// Release the input and both output references, then destroy/free the split filter.
void* gtl_CSplitFilter_dtor(SplitFilter* self, int deleting);

// Dispatch the split filter's input-reset and state-reset hooks in that order.
int gtl_CSplitFilter_ResetAndStart(CallbackObject* self);

// Return an added reference to the source sample when an output pointer is supplied.
int gtl_CSourceFilter_GetOutputSample(void* self, void* unused, unsigned int** out);

// Prepare an output buffer, validate formats once, and transform the pending input sample.
int gtl_CTransformFilter_Process(TransformFilterVirtuals* self);

// Read into an available pool sample, advance the byte offset, and preserve wait/error states.
int gtl_CSourceFilter_Process(SourceReaderVirtuals* self);

} // extern "C"

#pragma pop

// Definitions follow the original text-address order.
extern "C" {

#pragma push
void* GtlPagedQueue803F_Ctor(void* self) {
	unsigned int* fields = (unsigned int*)self;
	fields[0] = 0;
	fields[1] = 0;
	fields[2] = 0;
	fields[3] = 0;
	fields[4] = 0;
	fields[5] = 0;
	struct RangeVector { unsigned int* begin; unsigned int* end; unsigned int* capacity; } empty = { 0, 0, 0 };
	RangeVector* destination = (RangeVector*)((char*)self + 8);
	unsigned int count = (unsigned int)(empty.end - empty.begin);
	if (count != 0) {
		unsigned int* storage = (unsigned int*)gtl_Alloc((count + 1) * 4);
		destination->begin = storage;
		destination->end = storage + count + 1;
		destination->capacity = destination->end;
	}
	return self;
}
#pragma pop

#pragma push
unsigned int GtlPagedQueue803F_GetStorageEntryCapacity(StateValue* self) {
	return self->value == 0 ? 0 : (self->value - 1) << kStoredValueUnitShift;
}
#pragma pop

#pragma push
void* gtl_CSourceFilter_Ctor(SourceFilter* self, const SourceData* source, void* callback) {
	gtl_IFilter_Ctor(self, 0, callback);
	self->vtable = __vt__Q23gtl13CSourceFilter;
	self->readOffset = 0;
	self->outputSample = 0;
	self->samplePool = 0;
	self->readPending = 0;
	self->callback = callback;
	self->source = (void*)source;
	self->sourceValidated = 0;
	self->readOffset = source->firstReadOffset;
	return self;
}
#pragma pop

#pragma push
void* gtl_CSourceFilter_dtor(void* self, int deleting) {
	if (self != 0) {
		gtl_IFilter_dtor_Base(self, 0);
		if (deleting > 0 && self != 0)
			gtl_Free(self);
	}
	return self;
}
#pragma pop

#pragma push
int gtl_CSourceFilter_Process(SourceReaderVirtuals* self) {
	int state;
	gtl_IFilter_GetState(self, &state);
	if (state == kFilterStopped || state == kFilterOutputReady)
		return 1;
	if (Asgtl_CSourceFilter(self)->sourceValidated == 0) {
		if (!self->validateSource(Asgtl_CSourceFilter(self)->outputSample, 0)) {
			gtl_IFilter_SetState(self, kFilterFormatsRejected);
			return 0;
		}
		Asgtl_CSourceFilter(self)->sourceValidated = 1;
	}
	void* iterator = Asgtl_CSourceFilter(self)->source;
	if (((SourceData*)iterator)->byteLength <= Asgtl_CSourceFilter(self)->readOffset) {
		gtl_IFilter_SetState(self, kFilterOutputReady);
		return 1;
	}
	if (Asgtl_CSourceFilter(self)->samplePool == 0)
		return 0;
	if (SamplePool_IsEmpty(Asgtl_CSourceFilter(self)->samplePool)) {
		gtl_IFilter_SetState(self, kFilterWaitingForInput);
		return 1;
	}
	void* object;
	*(unsigned int*)&object = 0;
	if (!SamplePool_PeekAvailableSample(Asgtl_CSourceFilter(self)->samplePool, &object))
		return 1;
	unsigned int args[2];
	args[0] = kSourceReadBufferBytes;
	args[1] = kSourceReadAlignment;
	SamplePool_GetDimensions(Asgtl_CSourceFilter(self)->samplePool, args);
	switch (self->readSample(Asgtl_CSourceFilter(self)->callback, object, args)) {
	case kFilterOutputReady: {
		unsigned int amount;
		self->getBytesRead(&amount);
		SamplePool_CommitSample(Asgtl_CSourceFilter(self)->samplePool, amount);
		Asgtl_CSourceFilter(self)->readOffset += amount;
		gtl_IFilter_SetState(self, kFilterRunning);
		break;
	}
	case kFilterRunning:
		break;
	default:
		gtl_IFilter_SetState(self, kFilterProcessFailed);
		int refs = --*(int*)object;
		if (refs == 0 && object != 0)
			((RefObject*)object)->destroy(1);
		return 0;
	}
	int refs = --*(int*)object;
	if (refs == 0 && object != 0)
		((RefObject*)object)->destroy(1);
	return 1;
}
#pragma pop

#pragma push
int gtl_CSourceFilter_StartProcessing(void* self) {
	int value;
	gtl_IFilter_GetState(self, &value);
	if (value != kFilterOutputReady)
		gtl_IFilter_SetState(self, kFilterRunning);
	return 1;
}
#pragma pop

#pragma push
int gtl_CSourceFilter_ResumeUnlessOutputReady(void* self) {
	int value;
	gtl_IFilter_GetState(self, &value);
	if (value != kFilterOutputReady)
		gtl_IFilter_SetState(self, kFilterRunning);
	return 1;
}
#pragma pop

#pragma push
int gtl_CSourceFilter_ResetReadState(SourceResetState* self) {
	self->pendingRead = 0;
	self->readOffset = ((SourceData*)self->source)->firstReadOffset;
	gtl_IFilter_SetState(self, kFilterReset);
	return 1;
}
#pragma pop

#pragma push
int gtl_CSourceFilter_ResetAndStart(CallbackObject* self) {
	self->resetInput();
	self->resetState();
	return 1;
}
#pragma pop

#pragma push
int gtl_CSourceFilter_GetOutputSample(void* self, void* unused, unsigned int** out) {
	if (out == 0)
		return 0;
	unsigned int* value = (*(unsigned int**)&Asgtl_CSourceFilter(self)->outputSample);
	*out = value;
	(*value)++;
	return 1;
}
#pragma pop

#pragma push
int gtl_CSourceFilter_StoreInputSample(SourceInputState* self, void* input) {
	unsigned int dataAddress;
	self->input = input;
	gtl_CMediaSample_GetData(input, &dataAddress);
	self->bufferAddress = dataAddress;
	return 1;
}
#pragma pop

#pragma push
int gtl_CSourceFilter_vfunc_8(void* unused, int value) {
	return value != 0;
}
#pragma pop

#pragma push
int gtl_CSourceFilter_GetReadOffset(SourceReadCursor* self, unsigned int* out) {
	if (out == 0)
		return 0;
	*out = self->readOffset;
	return 1;
}
#pragma pop

#pragma push
unsigned char gtl_CSourceFilter_IsReadPending(SourceReadLatch* self) {
	return self->pendingRead;
}
#pragma pop

#pragma push
void gtl_CSourceFilter_MarkReadPending(SourceReadLatch* self) {
	self->pendingRead = 1;
}
#pragma pop

#pragma push
void gtl_CSourceFilter_ClearReadPending(SourceReadLatch* self) {
	self->pendingRead = 0;
}
#pragma pop

#pragma push
SplitFilter* gtl_CSplitFilter_Ctor(SplitFilter* self, void* input, void* arg2) {
	gtl_IFilter_Ctor(self, (void*)2, arg2);
	self->vtable = (void*)__vt__Q23gtl12CSplitFilter;
	self->inputSample = 0;
	self->formatsChecked = 0;
	void* first = gtl_Alloc(0x14);
	if (first != 0)
		gtl_CMediaSample_Ctor(first, 0, 0, input);
	self->primaryOutput = first;
	void* second = gtl_Alloc(0x14);
	if (second != 0)
		gtl_CMediaSample_Ctor(second, 0, 0, input);
	self->secondaryOutput = second;
	return self;
}
#pragma pop

#pragma push
void* gtl_CSplitFilter_dtor(SplitFilter* self, int deleting) {
	if (self != 0) {
		RefObject* input = (RefObject*)self->inputSample;
		self->vtable = __vt__Q23gtl12CSplitFilter;
		if (input != 0) {
			unsigned int refs = *(unsigned int*)input - 1;
			*(unsigned int*)input = refs;
			if (refs == 0)
				input->destroy(1);
			self->inputSample = 0;
		}
		RefObject* value = (RefObject*)self->primaryOutput;
		if (value != 0) {
			value->refs--;
			if (value->refs == 0)
				value->destroy(1);
			self->primaryOutput = 0;
		}
		value = (RefObject*)self->secondaryOutput;
		if (value != 0) {
			value->refs--;
			if (value->refs == 0)
				value->destroy(1);
			self->secondaryOutput = 0;
		}
		gtl_IFilter_dtor_Base(self, 0);
		if (deleting > 0 && self != 0)
			gtl_Free(self);
	}
	return self;
}
#pragma pop

#pragma push
int gtl_CSplitFilter_Process(FilterVTable8808* self) {
	int state;
	gtl_IFilter_GetState(self, &state);
	if (state == kFilterStopped || state == kFilterReset)
		return 1;
	void* input = Asgtl_CSplitFilter(self)->inputSample;
	if (input == 0) {
		gtl_IFilter_SetState(self, kFilterWaitingForInput);
		return 1;
	}
	if (Asgtl_CSplitFilter(self)->formatsChecked == 0) {
		void* first = Asgtl_CSplitFilter(self)->primaryOutput;
		if (!self->formatsMatch(input, first) ||
		    !self->formatsMatch(input, Asgtl_CSplitFilter(self)->secondaryOutput)) {
			gtl_IFilter_SetState(self, kFilterFormatsRejected);
			return 0;
		}
		Asgtl_CSplitFilter(self)->formatsChecked = 1;
	}
		int result = self->splitSample(Asgtl_CSplitFilter(self)->inputSample, Asgtl_CSplitFilter(self)->primaryOutput, Asgtl_CSplitFilter(self)->secondaryOutput);
	if (result != kFilterOutputReady) {
		gtl_IFilter_SetState(self, kFilterProcessFailed);
		return 0;
	}
	gtl_IFilter_SetState(self, kFilterOutputReady);
	return 1;
}
#pragma pop

#pragma push
int gtl_CSplitFilter_SetRunning(void* self) {
	gtl_IFilter_SetState(self, kFilterRunning);
	return 1;
}
#pragma pop

#pragma push
int gtl_CSplitFilter_Stop(void* self) {
	gtl_IFilter_SetState(self, kFilterStopped);
	return 1;
}
#pragma pop

#pragma push
int gtl_CSplitFilter_Reset(void* self) {
	void* value = Asgtl_CSplitFilter(self)->inputSample;
	if (value != 0) {
		unsigned int count = *(unsigned int*)value - 1;
		*(unsigned int*)value = count;
		if (count == 0) {
			if (value != 0)
				((RefObject*)value)->destroy(1);
		}
		Asgtl_CSplitFilter(self)->inputSample = 0;
	}
	gtl_IFilter_SetState(self, kFilterReset);
	return 1;
}
#pragma pop

#pragma push
int gtl_CSplitFilter_ResetAndStart(CallbackObject* self) {
	self->resetInput();
	self->resetState();
	return 1;
}
#pragma pop

#pragma push
int gtl_CSplitFilter_GetOutputSample(void* self, int which, unsigned int** out) {
	if (out == 0)
		return 0;
	if (which == 0) {
		unsigned int* value = (*(unsigned int**)&Asgtl_CSplitFilter(self)->primaryOutput);
		*out = value;
		(*value)++;
	} else if (which == 1) {
		unsigned int* value = (*(unsigned int**)&Asgtl_CSplitFilter(self)->secondaryOutput);
		*out = value;
		(*value)++;
	}
	return 1;
}
#pragma pop

#pragma push
int gtl_CSplitFilter_SetInputSample(void* self, RefObject* value) {
	RefObject* old;
	old = (*(RefObject**)&Asgtl_CSplitFilter(self)->inputSample);
	if (old != 0) {
		old->refs--;
		if (old->refs == 0)
			old->destroy(1);
		(*(RefObject**)&Asgtl_CSplitFilter(self)->inputSample) = 0;
	}
	(*(RefObject**)&Asgtl_CSplitFilter(self)->inputSample) = value;
	value->refs++;
	return 1;
}
#pragma pop

#pragma push
int gtl_CSplitFilter_FormatsMatch(void* unused, void* left, void* right) {
	if (left == 0 || right == 0)
		return 0;
	// gtl_CMediaSample_CopyFormat fills a 0xA4-byte description.
	char leftData[0xa4];
	char rightData[0xa4];
	gtl_CMediaSample_CopyFormat(left, leftData);
	gtl_CMediaSample_CopyFormat(right, rightData);
	return gtl_MediaFormat_Equals(leftData, rightData);
}
#pragma pop

#pragma push
TransformFilter* gtl_CTransformFilter_Ctor(TransformFilter* self, void* arg1, void* arg2) {
	gtl_IFilter_Ctor(self, arg1, arg2);
	self->inputSample = 0;
	self->vtable = __vt__Q23gtl16CTransformFilter;
	self->outputSample = arg2;
	self->formatsChecked = 0;
	return self;
}
#pragma pop

#pragma push
void* gtl_CTransformFilter_dtor(TransformFilter* self, int deleting) {
	if (self != 0) {
		void* input = self->inputSample;
		self->vtable = __vt__Q23gtl16CTransformFilter;
		if (input != 0) {
			unsigned int refs = *(unsigned int*)input - 1;
			*(unsigned int*)input = refs;
			if (refs == 0)
				((RefObject*)input)->destroy(1);
		self->inputSample = 0;
		}
		gtl_IFilter_dtor_Base(self, 0);
		if (deleting > 0 && self != 0)
			gtl_Free(self);
	}
	return self;
}
#pragma pop

#pragma push
int gtl_CTransformFilter_Process(TransformFilterVirtuals* self) {
	int state;
	gtl_IFilter_GetState(self, &state);
	if (state == kFilterStopped || state == kFilterReset)
		return 1;
	// The original re-reads the input at 0x14 around each call.
	unsigned int ready;
	gtl_CMediaSample_GetData(Asgtl_CTransformFilter(self)->outputSample, &ready);
	if (!ready) {
		char description[0xa4]; // gtl_CMediaSample_CopyFormat fills 0xA4 bytes
		gtl_CMediaSample_CopyFormat(Asgtl_CTransformFilter(self)->outputSample, description);
		unsigned int second;
		self->allocateOutputBuffer(&ready, &second);
		gtl_CMediaSample_SetBuffer(Asgtl_CTransformFilter(self)->outputSample, ready, second);
	}
	if (Asgtl_CTransformFilter(self)->inputSample == 0) {
		gtl_IFilter_SetState(self, kFilterWaitingForInput);
		return 1;
	}
	if (Asgtl_CTransformFilter(self)->formatsChecked == 0) {
		if (!self->formatsMatch(Asgtl_CTransformFilter(self)->inputSample, Asgtl_CTransformFilter(self)->outputSample)) {
			gtl_IFilter_SetState(self, kFilterFormatsRejected);
			return 0;
		}
		Asgtl_CTransformFilter(self)->formatsChecked = 1;
	}
	unsigned int pending;
	gtl_CMediaSample_GetSize(Asgtl_CTransformFilter(self)->outputSample, &pending);
	if (!pending)
		return 1;
	int result = self->transformSample(Asgtl_CTransformFilter(self)->inputSample, Asgtl_CTransformFilter(self)->outputSample);
	if (result == kFilterProcessFailed) {
		gtl_IFilter_SetState(self, kFilterProcessFailed);
		return 0;
	}
	gtl_IFilter_SetState(self, kFilterOutputReady);
	return 1;
}
#pragma pop

#pragma push
int gtl_CTransformFilter_ResumeOrExposeOutput(void* self) {
	int value;
	gtl_IFilter_GetState(self, &value);
	if (value == kFilterStopped)
		gtl_IFilter_SetState(self, kFilterOutputReady);
	else
		gtl_IFilter_SetState(self, kFilterRunning);
	return 1;
}
#pragma pop

}
