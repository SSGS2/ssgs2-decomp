// Translation unit 0x803E78EC-0x803E8244 (12 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".
#include "sdk_protos.h"

// gtl utility API bundle (no gameplay tuning):
// - Model/texture transforms: BuildNodeMatrix/BuildTextureMatrix; the transform
//   record's mode changes rotation REPRESENTATION, not just multiplication order.
// - Camera/frustum: BuildPerspectivePlanes writes six view-space planes.
// - Bounds: ProjectBoxRadius projects three box half-axis vectors onto a direction.
// - Timing: Stopwatch_GetElapsedSeconds consumes the time since the LAST query.
// - Controller setup: CInputExDeviceBase_Initialize and its enumeration callback;
//   configuration comes from InputEx_GetButtonConfiguration, not constants here.
// Matrix helpers live in gtl_vector_matrix_math.cpp: contiguous 4x4 float rows, row-vector
// multiplication, translation at [12..14]; Concat3 is (a*b)*c. Euler angles and
// fovY are radians. Model translation retains caller coordinate/distance units;
// texture translation is in texture-coordinate space (no pixel conversion).

struct GtlInputExInitParams {
    void* device;
    int descriptor;
    void* buttonTable;
};

struct GtlInputExButtonConfig {
    float threshold;
    int repeatInterval;
    int repeatDelay;
};

struct GtlInputExHeader {
    int unk0;
};

struct GtlInputExDeviceBase : GtlInputExHeader {
    virtual void vfunc0();
    virtual void vfunc1();
    virtual void vfunc2();
    virtual void* getButton(unsigned int index);
    virtual int getCount();
    void* device;
    unsigned char initialized;
    char reservedD[3];
    void* buttonTable;
};

extern "C" void* s_activeInputDevice;
extern "C" void* s_activeInputExInitDescriptor;
extern "C" const float kFrustumCotangentNumerator;
extern "C" const float kFloat_0_Frustum;
extern "C" const float kHalfFieldOfViewScale;
extern "C" const float kFrustumPlaneNegativeZ;

struct GtlStopwatch {
    unsigned char valid; // frequency-query success; gates every operation
    unsigned char accumulateOnStart; // initialized 1; Start/Reset clear it
    char unk2[6];
    long long frequency; // ticks/second
    long long heldTick; // nonzero overrides current timer reads; zero means live
    long long lastTick; // baseline advanced by Start/Reset/GetElapsedSeconds
    long long storedTicks; // Start adds (now-heldTick); Reset assigns a timestamp
};

struct GtlMtx44 { float m[4][4]; };

struct GtlRefCountedBase {
    int refCount;
};

struct GtlRefCounted : GtlRefCountedBase {
    virtual ~GtlRefCounted();
};

struct GtlInputExDeviceDtorView {
    int unk0;
    void* vtable;
    GtlRefCounted* device;
};

struct GtlTextureTransform {
    float scale[3];       // +0: modes 0..5 use XY; mode 6 also reads Z
    char unkC[4];
    float rotation[4];    // +0x10: mode-6 XYZW quaternion
    float translation[3];// +0x20: modes 0..5 use XY; mode 6 also reads Z
    int mode;            // +0x2C: 0..5 planar scale/translation; 6 full transform
};

struct GtlNodeTransform {
    float scale[3];
    char unkC[4];
    float rotation[4]; // +0x10: mode 5 XYZ Euler radians; mode 6 XYZW quaternion
    float translation[3];
    int mode;
};

extern "C" {

int gtl_InputEx_EnumerateItemCallback();
int gtl_IInputDevice_SetDescriptor(void* device, int value);
int InputDevice_NoOpSuccess_80435FB4(void* device, void* a, int b);
int InputDevice_NoOpSuccess_80435D34(void* device);
void* gtl_IInputDevice_EnumerateItems(void* device, void* a, void* b);
int gtl_IInputDevice_GetProviderId(void* device, int* providerOut);
GtlInputExButtonConfig* gtl_InputEx_GetButtonConfiguration(int providerId, unsigned int index, void* table);
void gtl_IInputExButton_SetThreshold(void* button, float value);
void gtl_IInputExButton_SetRepeatInterval(void* button, int value);
void gtl_IInputExButton_SetRepeatDelay(void* button, int value);
float Vec3_Length(void* v);

int Timer_GetTickFrequency(long long* out);
GtlStopwatch* gtl_Stopwatch_Initialize(GtlStopwatch* self);

extern char __vt__Q23gtl18CInputExDeviceBase[];
void __dl__Q23gtl25IInputResourceMemOperatorFPv(void* p);
void Timer_ReadQuantizedTickCount(long long* out);
void gtl_Stopwatch_Start(GtlStopwatch* self);

void gtl_Stopwatch_Reset(GtlStopwatch* self);

extern "C" const float kStopwatchInvalidElapsed;
float gtl_Stopwatch_GetElapsedSeconds(GtlStopwatch* self);

int gtl_InputEx_EnumerateItemCallback();

void* __dt__Q23gtl18CInputExDeviceBaseFv(GtlInputExDeviceDtorView* self, int flags);

int gtl_CInputExDeviceBase_Initialize(GtlInputExDeviceBase* self, const GtlInputExInitParams* params);

void Mtx44_SetRotationX(GtlMtx44* m, float angle);
void Mtx44_SetRotationY(GtlMtx44* m, float angle);
void Mtx44_SetRotationZ(GtlMtx44* m, float angle);
void Mtx44_Concat3(GtlMtx44* out, const GtlMtx44* a, const GtlMtx44* b, const GtlMtx44* c);
void Quat_FromMatrix44(void* q, const GtlMtx44* m);
void* gtl_Transform_QuaternionFromEuler(void* out, float z, float x, float y);

void PSMTX44Identity(float* m);
void Mtx44_BuildTransformation(float* m, const void* a, const void* b, const void* scale, const void* c, const void* rotation, const void* translation);
float* gtl_Transform_BuildTextureMatrix(float* m, const GtlTextureTransform* t);

float Vec3_Dot(const void* a, const void* b);
float gtl_Bounds_ProjectBoxRadius(const float* axes, const void* dir, int flag);

void gtl_Frustum_BuildPerspectivePlanes(float* planes, float fovY, float aspect, float nearZ, float farZ);

// Required writable out[4], XYZW quaternion; no ownership transfer. ABI argument
// order is Z,X,Y, but composition is (Rx*Ry)*Rz. Inputs are Euler radians.
void* gtl_Transform_QuaternionFromEuler(void* out, float z, float x, float y) {
    GtlMtx44 r;
    GtlMtx44 rz;
    GtlMtx44 ry;
    GtlMtx44 rx;
    Mtx44_SetRotationX(&rx, x);
    Mtx44_SetRotationY(&ry, y);
    Mtx44_SetRotationZ(&rz, z);
    Mtx44_Concat3(&r, &rx, &ry, &rz);
    Quat_FromMatrix44(out, &r);
    return out;
}

void Mtx44_SetScale(GtlMtx44* m, float x, float y, float z);
void Mtx44_SetTranslation(GtlMtx44* m, float x, float y, float z);
void PSMTX44Concat(const float* a, const float* b, float* ab);
// Required m[16] and borrowed t, disjoint. Mode 5 writes S*(Rx*Ry*Rz)*T;
// mode 6 writes S*R(quaternion)*T via BuildTransformation with all centers null.
// Modes 0..4 and unknown modes leave ALL of m unchanged (not identity).
// Returns m; no validation, allocation, normalization or failure result.
float* gtl_Transform_BuildNodeMatrix(float* m, const GtlNodeTransform* t) {
    GtlMtx44 rz;
    GtlMtx44 ry;
    GtlMtx44 rx;
    GtlMtx44 tmp;
    switch (t->mode) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
        break;
    case 5:
        PSMTX44Identity(m);
        Mtx44_SetScale(&tmp, t->scale[0], t->scale[1], t->scale[2]);
        PSMTX44Concat(m, m, tmp.m[0]);
        Mtx44_SetRotationX(&rx, t->rotation[0]);
        Mtx44_SetRotationY(&ry, t->rotation[1]);
        Mtx44_SetRotationZ(&rz, t->rotation[2]);
        Mtx44_Concat3(&tmp, &rx, &ry, &rz);
        PSMTX44Concat(m, m, tmp.m[0]);
        Mtx44_SetTranslation(&tmp, t->translation[0], t->translation[1], t->translation[2]);
        PSMTX44Concat(m, m, tmp.m[0]);
        break;
    case 6:
        Mtx44_BuildTransformation(m, 0, 0, t, 0, t->rotation, t->translation);
        break;
    }
    return m;
}

// Required m[16] and borrowed t, disjoint; always initializes identity first.
// Modes 0..5 set [0]/[5] scales and [8]/[9] offsets: this planar layout is not
// the node matrix's [12..14] translation. Mode 6 uses all XYZ scale/translation
// components and an XYZW quaternion. Unknown modes leave the written identity.
// Returns m; no allocation or failure code. Do not reinterpret mode 6 as Euler.
float* gtl_Transform_BuildTextureMatrix(float* m, const GtlTextureTransform* t) {
    PSMTX44Identity(m);
    switch (t->mode) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
        m[0] = t->scale[0];
        m[5] = t->scale[1];
        m[8] = t->translation[0];
        m[9] = t->translation[1];
        break;
    case 6:
        Mtx44_BuildTransformation(m, 0, 0, t, 0, t->rotation, t->translation);
        break;
    }
    return m;
}

// axes borrows three half-axis XYZ vectors at floats 0,4,8 (four-float stride),
// dir borrows XYZ in the SAME space. With unit dir, return projected half-extent
// in axis distance units; otherwise it is also scaled by |dir|. flag==0 sums
// absolute dot products in the original order; every other flag returns 0.
float gtl_Bounds_ProjectBoxRadius(const float* axes, const void* dir, int flag) {
    switch (flag) {
    case 0: {
        double a = __fabs((double)Vec3_Dot(axes + 4, dir));
        a = (float)a;
        double b = __fabs((double)Vec3_Dot(axes, dir));
        b = (float)b;
        float c = (float)__fabs((double)Vec3_Dot(axes + 8, dir));
        return c + ((float)b + (float)a);
    }
    }
    return kFloat_0_Frustum;
}

// Required nonaliasing planes[24]: six {nx,ny,nz,d} records, evaluated as
// nx*x+ny*y+nz*z+d >= 0 INSIDE, for a view looking down -Z. Order:
// near {0,0,-1,-nearZ}, far {0,0,1,farZ}, left, right, bottom, top.
// Side normals are normalized independently; their d==0. nearZ/farZ are positive
// view-space distances with farZ>nearZ, aspect=width/height>0, 0<fovY<pi radians.
// No checks: degenerate inputs divide by zero/produce nonfinite coefficients.
// Returns nothing; writes progressively; no allocation or retained input.
void gtl_Frustum_BuildPerspectivePlanes(float* planes, float fovY, float aspect, float nearZ, float farZ) {
    float cot = kFrustumCotangentNumerator / (float)tan(kHalfFieldOfViewScale * fovY);
    float len;
    float* q;
    float* p;

    planes[3] = -nearZ;
    planes[0] = kFloat_0_Frustum;
    planes[1] = kFloat_0_Frustum;
    planes[2] = kFrustumPlaneNegativeZ;
    p = planes + 8;
    planes[4] = kFloat_0_Frustum;
    planes[5] = kFloat_0_Frustum;
    planes[6] = kFrustumCotangentNumerator;
    planes[7] = farZ;
    planes[9] = kFloat_0_Frustum;
    planes[8] = cot / aspect;
    planes[10] = kFrustumPlaneNegativeZ;
    planes[11] = kFloat_0_Frustum;
    len = Vec3_Length(p);
    q = planes + 12;
    p[0] = p[0] / len;
    p[1] = p[1] / len;
    p[2] = p[2] / len;
    p[3] = p[3] / len;
    planes[12] = -cot / aspect;
    planes[13] = kFloat_0_Frustum;
    planes[14] = kFrustumPlaneNegativeZ;
    planes[15] = kFloat_0_Frustum;
    len = Vec3_Length(q);
    p = planes + 16;
    planes[18] = kFrustumPlaneNegativeZ;
    planes[16] = kFloat_0_Frustum;
    planes[19] = kFloat_0_Frustum;
    planes[17] = cot;
    q[0] = q[0] / len;
    q[1] = q[1] / len;
    q[2] = q[2] / len;
    q[3] = q[3] / len;
    len = Vec3_Length(p);
    q = planes + 20;
    planes[21] = -cot;
    planes[22] = kFrustumPlaneNegativeZ;
    planes[20] = kFloat_0_Frustum;
    p[0] = p[0] / len;
    planes[23] = kFloat_0_Frustum;
    p[1] = p[1] / len;
    p[2] = p[2] / len;
    p[3] = p[3] / len;
    len = Vec3_Length(q);
    q[0] = q[0] / len;
    q[1] = q[1] / len;
    q[2] = q[2] / len;
    q[3] = q[3] / len;
}

// Initialize caller storage, query tick frequency, return self even on failure.
// Timer_GetTickFrequency must write freq even on failure: it is copied regardless.
// Success requires positive frequency. valid==0 makes later mutators no-ops and
// GetElapsedSeconds return -1. Unknown reserved bytes remain untouched.
GtlStopwatch* gtl_Stopwatch_Initialize(GtlStopwatch* self) {
    long long freq;
    self->valid = 0;
    self->accumulateOnStart = 1;
    self->frequency = 0;
    self->heldTick = 0;
    self->lastTick = 0;
    self->storedTicks = 0;
    self->valid = Timer_GetTickFrequency(&freq) != 0;
    self->frequency = freq;
    return self;
}

// Sample live time and rebase the next elapsed query. If accumulateOnStart is
// set, add now-heldTick to storedTicks; then clear heldTick and that flag.
// This is not a total-time getter; storedTicks is not read by GetElapsedSeconds.
void gtl_Stopwatch_Start(GtlStopwatch* self) {
    if (self->valid) {
        long long now;
        Timer_ReadQuantizedTickCount(&now);
        if (self->accumulateOnStart) {
            self->storedTicks += now - self->heldTick;
        }
        self->heldTick = 0;
        self->lastTick = now;
        self->accumulateOnStart = 0;
    }
}

// Rebase storedTicks AND lastTick to heldTick if nonzero, otherwise live time.
// Clear heldTick and accumulateOnStart. Despite the ABI name, this does not
// zero every timestamp; the next elapsed query measures time since this baseline.
void gtl_Stopwatch_Reset(GtlStopwatch* self) {
    if (self->valid) {
        long long now;
        if (self->heldTick != 0) {
            now = self->heldTick;
        } else {
            Timer_ReadQuantizedTickCount(&now);
        }
        self->storedTicks = now;
        self->lastTick = now;
        self->heldTick = 0;
        self->accumulateOnStart = 0;
    }
}

#pragma push
#pragma opt_propagation off
// Return (current-or-held tick - lastTick)/frequency in float SECONDS, then
// advance lastTick. Consecutive queries consume intervals, not time since Start.
// Required initialized self; caller serializes queries/mutations. Invalid returns
// -1 without writes. Keep integer subtraction before separate float conversions.
float gtl_Stopwatch_GetElapsedSeconds(GtlStopwatch* self) {
    if (!self->valid) {
        return kStopwatchInvalidElapsed;
    }
    long long tick;
    long long now;
    if (self->heldTick != 0) {
        tick = self->heldTick;
    } else {
        Timer_ReadQuantizedTickCount(&tick);
    }
    now = tick;
    float freq = (float)self->frequency;
    float result = (float)(now - self->lastTick) / freq;
    self->lastTick = now;
    return result;
}
#pragma pop

// Enumeration admission hook always accepts; device publication is external.
int gtl_InputEx_EnumerateItemCallback() {
    return 1;
}

// Release one retained device reference (delete at zero), then free self only
// for flags>0. Null self is accepted; return may dangle after a deleting call.
void* __dt__Q23gtl18CInputExDeviceBaseFv(GtlInputExDeviceDtorView* self, int flags) {
    if (self != 0) {
        GtlRefCounted* d = self->device;
        self->vtable = __vt__Q23gtl18CInputExDeviceBase;
        if (d != 0 && --d->refCount == 0) {
            delete d;
        }
        if (flags > 0) {
            __dl__Q23gtl25IInputResourceMemOperatorFPv(self);
        }
    }
    return self;
}

// Required self/params/device; buttonTable must match provider/index lookup.
// Pointers are installed without AddRef here: caller must supply the reference
// later consumed by the destructor. Configuration threshold is a raw button
// value; interval/delay pass through in the button API's units (not established
// as seconds here). getButton must return valid buttons for [0,getCount()).
// No rollback: descriptor/device setup, global s_activeInputDevice publication, item
// enumeration and earlier button settings may survive a zero return.
// Preserve the unusual outcomes: all configurations present -> return 0;
// first missing config -> finish device setup, return 1 even if that call fails;
// initialized is set to 1 only when that final call succeeds, never cleared.
// Initialize may therefore leave an old initialized value on any failure path.
int gtl_CInputExDeviceBase_Initialize(GtlInputExDeviceBase* self, const GtlInputExInitParams* params) {
    self->device = params->device;
    self->buttonTable = params->buttonTable;
    if (!gtl_IInputDevice_SetDescriptor(params->device, params->descriptor)) {
        return 0;
    }
    if (!InputDevice_NoOpSuccess_80435FB4(self->device, ((void**)s_activeInputExInitDescriptor)[2], 6)) {
        return 0;
    }
    s_activeInputDevice = self->device;
    if (!gtl_IInputDevice_EnumerateItems(self->device, (void*)gtl_InputEx_EnumerateItemCallback, 0)) {
        return 0;
    }
    unsigned int count = self->getCount();
    int providerId;
    gtl_IInputDevice_GetProviderId(self->device, &providerId);
    void* button;
    unsigned int index = 0;
    int allButtonsConfigured;
    while (index < count) {
        button = self->getButton(index);
        GtlInputExButtonConfig* config = gtl_InputEx_GetButtonConfiguration(providerId, index, self->buttonTable);
        if (!config) {
            allButtonsConfigured = 0;
            goto finishButtonConfiguration;
        }
        gtl_IInputExButton_SetThreshold(button, config->threshold);
        gtl_IInputExButton_SetRepeatInterval(button, config->repeatInterval);
        gtl_IInputExButton_SetRepeatDelay(button, config->repeatDelay);
        index++;
    }
    allButtonsConfigured = 1;
finishButtonConfiguration:
    if (allButtonsConfigured) {
        return 0;
    }
    if (InputDevice_NoOpSuccess_80435D34(self->device)) {
        self->initialized = 1;
    }
    return 1;
}

} // extern "C"
