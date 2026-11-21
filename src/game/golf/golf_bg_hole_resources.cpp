enum { kVariantCourseModelPath = 8, kWeatherResourceSuffixCount = 3,
    kPartyVariantCourseMode = 2, kAimPathEntryKind = 4, kDefaultShadowCourseId = 0xFD };
enum GolfBackgroundTextOffsets {
    kCourseModelPath = 0x18,
    kVariantListStemFormat = 0x25,
    kVariantListSuffix = 0x2f,
    kListSeparator = 0x33,
    kListZipSuffix = 0x35,
    kListStemFormat = 0x3f,
    kGolfRuleActor = 0x47,
    kPrimaryGridPoint = 0x50,
    kAlternateGridPoint = 0x59,
    kSunName = 0x63
};

#include "classes/CSharedDoc.h"
#include "classes/CourseData801C.h"
#include "iff/IffStruct.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "classes/CGolfDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
// Translation unit 0x80121B14-0x801228E8 (18 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include "game/types.h"
#include "sdk_protos.h"

extern "C" {

extern unsigned int g_shadowManager;
extern void* g_golfCupPreview;
extern void* g_soundManager;
extern void* g_fx;
extern signed char s_renderPacketInitialized;
extern void* g_taskManager;
extern void* g_sharedDoc;
extern void* Object_GetChild(void*);
extern void RenderPacket_Init(void*);
extern void appFreeUni__FPv(void*);
extern void SoundManager_ReleaseSoundBank(void*, void*);
extern void __dl__FPv(void*);
extern void* RenderPacket_Destroy(void*, int);
extern char s_golfBackgroundRenderPacketDestructorRegistration[];
extern char s_renderPacket[];
extern char golfBackgroundStrings[];

extern void* GolfDoc_GetCurrent();
extern void* GolfPlayer_GetCurrentEntryByIndex(unsigned char);
extern unsigned char SharedDoc_GetGolfRuleType();
extern void* CGolfDoc_RestoreFirstPlayerSnapshot(void*);
extern void CourseData_BuildQuadTree(void*);
extern unsigned char GolfBg_DispatchCameraMessage2(void*);
extern void* CourseData_FindNamedPoint(void*, const char*);
extern void CGolfBg_GroundAndDispatchAmbientPosition(void*, void*);
extern void CGolfDoc_SetCurrentHoleStartOnGround(void*, void*);
extern void CGolfDoc_SetCurrentHoleCupOnGround(void*, void*);
extern void CPvsFxBox_ReloadCourseEffects(void*);
extern void CPvsFxBox_RebuildAuxiliaryEffect(void*);
extern void CPvsFxBox_RebuildSceneEffects(void*);
extern void* CShadowManager_CopyNamedRecordSettings(void*, const char*, void*);
extern void* GolfTask_NoOpStatus();
extern void* GolfBall__Fv();
extern void __pl__FRC7WVectorRC7WVector(void*, void*, void*);
extern void CGolfBall_Reset(void*, void*);
extern void* SceneManager_GetCurrent();
extern void CGolfBg_CreateCourseObjects(void*);
extern void GolfAimPath_Clear(void*);
extern void CourseEntryIterator_DefaultConstruct(void*);
extern void* CourseEntryVector_Begin(void*);
extern void* CourseEntryIterator_CopyConstruct(void*, void*, int);
extern void CourseEntryIterator_Assign(void*, void*);
extern void* CourseEntryIterator_GetEntry(void*);
extern void* CourseEntry_GetPointVector(void*);
extern void Vec3Vector_CopyAppendAndFlattenHeight(void*, void*, void*);
extern void CourseEntryIterator_PostIncrement(void*, int);
extern void* CourseEntryVector_End(void*);
extern int CourseEntryIterator_NotEqual(void*, void*);
extern void CourseData_ReleaseResources(void*, int);
extern unsigned char SharedDoc_GetLastRoundHoleIndex();
extern unsigned char SharedDoc_GetRoundHoleCount();
extern unsigned char CGolfDoc_GetPlayerCount(void*);
extern void IActor_DispatchMessage(void*, void*, const char*, int, int, int, int, int);
extern void SoundManager_StopAndClearChannels(void*);
extern void CFx_SetUpdateEnabled(void*, int);
extern void CFx_ClearControlsAndResetBox(void*);
extern void SharedDoc_SelectCourse(int);
extern void* String_FormatTemporary(const char*, ...);
extern int CHashPath_FindPath(void*);
extern void String_ReserveCapacity(void*, unsigned int);
extern void String_ReplaceRange(void*, int, int, const char*, const char*, const void*);
extern int CourseModel_LoadFromFile(void*, void*);
extern void CPvsFxBox_LoadSkyResources(void*, void*, float);
extern void CPvsFxBox_LoadVariantBitmap(void*, void*);
extern void* CPackedFile_Create(const char*);
extern int CPackedFile_PollLoad(void*);
extern void CGolfBg_InitializeCurrentHole(void*);
extern void CupPreview_SetPosition(void*, void*);
extern void ShadowManager_SetCaptureQuadAlphaPreset(unsigned int, int, int);
extern unsigned char g_courseLoadFailed;
extern unsigned char g_courseResourceListsEnabled;
extern const char* s_courseWeatherResourceSuffixes[4];
extern float kHoleSetupZero;

// Bit-preserving view of three world-space floats; the integer member types
// reproduce word copies and must not be used for numeric vector arithmetic.
struct Vector3 {
	unsigned int x;
	int y;
	unsigned int z;
};

// Views for fields still in header padding. Hole entries are 0x20 bytes, including tee/cup and par.
struct HoleRecord {
    Vector3 tee;
    Vector3 cup;
    unsigned char par;
    char remainingHoleData[7];
};
struct HoleEnvironment {
    char precedingDocumentFields[0x36C];
    unsigned int weatherByHole[18];
    char flags[2];
    unsigned char holeIndex;
    unsigned char holeNumber;
};
struct RoundCourseSelection {
    char precedingSharedFields[0x83D];
    unsigned char courseIds[18];
};
struct SharedCourseResources {
    char precedingSharedFields[0x9C6];
    unsigned char selectedWeather;
    char precedingCourseName[0x1D];
    char courseName[32];
};
struct MatchPlayerRecord {
    char precedingPlayerFields[0xC8];
    int holesWon;
    char precedingMatchState[0x94];
    int matchPointState;
};
enum MatchPointState { kNoMatchPoint = 0, kOneShortOfDormie = 1, kDormie = 2 };
enum GolfRuleKind { kMatchPlay = 1 };
// Dormie means the lead equals the holes still to play, including the current hole.
// Metrowerks byte-string storage: high flag bit owns heap; otherwise the first
// byte holds the length and text starts at +1. Raw view has no automatic cleanup.
// Returned data pointers are borrowed until the string changes or is destroyed.
struct TinyString {
 unsigned int flags;
 unsigned int length;
 char* heap;
 // Release heap text; inline text belongs to the string object.
 ~TinyString() { if(flags >> 31) appFreeUni__FPv(heap); }
 // Return text length in bytes, excluding the terminator.
 unsigned int size() const { if((flags >> 31)==0) return *(const unsigned char*)this & 0x7f; else return length; }
 // Borrow inline or heap text according to the ownership bit.
 char* data() { if((flags >> 31)==0) return (char*)this+1; else return heap; }
};
struct TinyStringRaw {
 unsigned int flags;
 unsigned int length;
 char* heap;
 // Return text length in bytes, excluding the terminator.
 unsigned int size() const { if((flags >> 31)==0) return *(const unsigned char*)this & 0x7f; else return length; }
 // Borrow inline or heap text according to the ownership bit.
 char* data() { if((flags >> 31)==0) return (char*)this+1; else return heap; }
};
struct Tag_80121C20 { char value; char pad[3]; };
struct RuntimeObject {
 virtual void destroy(int);
 virtual void slotC();
 virtual void slot10();
 virtual void slot14();
 virtual void slot18();
 virtual void slot1C();
 virtual void slot20();
 virtual void slot24();
 virtual void slot28();
 virtual void activate();
};


#pragma dont_inline on

#pragma dont_inline off
// Requires a selected course, live document/effect/task managers and valid hole
// tables. This is synchronous: archive polling may block until loading completes.
// The archive handle is released on normal completion. Preserve the existing
// early failure path: sets g_courseLoadFailed and returns before scene rebuild
// or archive cleanup; do not add cleanup to matching code.
// Load this hole's course/variant resources, rebuild scene objects, and update cup preview and shadows.
void CGolfBg_ReloadCurrentHoleResources(void* self) {
 unsigned int appendPosition;
 // Phase 1: notify listeners, stop audio/effects and select this hole's course/weather.
 IActor_DispatchMessage(self,self,golfBackgroundStrings,2,0,0,0,0);
 SoundManager_StopAndClearChannels(g_soundManager);
 if (g_fx) { CFx_SetUpdateEnabled(g_fx,0); CFx_ClearControlsAndResetBox(g_fx); }
 unsigned int courseId=((RoundCourseSelection*)g_sharedDoc)->courseIds[AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->currentHoleIndex];
 SharedDoc_SelectCourse(courseId);
 int environmentChanged=0;
 if(AsCSharedDoc(g_sharedDoc)->gameMode<=1) {
   CSharedDoc* root=AsCSharedDoc(g_sharedDoc);
   CGolfDoc* golfDocument=AsCGolfDoc((void*)root->golfDoc);
   unsigned char previousWeather=((SharedCourseResources*)root)->selectedWeather;
   unsigned int index=((HoleEnvironment*)golfDocument)->weatherByHole[golfDocument->currentHoleIndex];
   ((SharedCourseResources*)root)->selectedWeather=index;
   environmentChanged=index!=previousWeather;
 }
 // Phase 2: prefer a party variant model path if it exists, then use the base path.
 void* courseModelPath=0;
 if ((int)AsCSharedDoc(g_sharedDoc)->partyGameMode==kPartyVariantCourseMode) {
     CSharedDoc* root=AsCSharedDoc(g_sharedDoc);
     __typeof__(root->selectedCourseData) courseRecordForType = root->selectedCourseData;
     __typeof__(kVariantCourseModelPath+golfBackgroundStrings) variantPathFormat = kVariantCourseModelPath+golfBackgroundStrings;
     __typeof__(root->selectedCourseData) selectedCourseRecord = root->selectedCourseData;
     __typeof__(courseRecordForType) courseRecord = selectedCourseRecord;
     courseModelPath=String_FormatTemporary(variantPathFormat,root->selectedCourseData?((IFF_STRUCT::sCourse*)courseRecord)->mapName:0,((HoleEnvironment*)root->golfDoc)->holeNumber);
     if (!CHashPath_FindPath(courseModelPath)) courseModelPath=0;
 }
 if (courseModelPath == 0) courseModelPath=String_FormatTemporary(golfBackgroundStrings + kCourseModelPath,AsCSharedDoc(g_sharedDoc)->selectedCourseData?((IFF_STRUCT::sCourse*)AsCSharedDoc(g_sharedDoc)->selectedCourseData)->mapName:0,((HoleEnvironment*)AsCSharedDoc(g_sharedDoc)->golfDoc)->holeNumber);
 void* resourceListArchive=0;
 TinyStringRaw resourceListPath;
 Vector3 cupPositionBits;
 // Phase 3: build the optional weather-suffixed archive path. The variant stem
 // is probed first; missing variant paths fall back to the ordinary list stem.
 if(g_courseResourceListsEnabled) {
   Tag_80121C20 srcF;
   Tag_80121C20 cpyF;
   Tag_80121C20 srcG;
   Tag_80121C20 cpyG;
   Tag_80121C20 srcH;
   Tag_80121C20 cpyH;
   Tag_80121C20 cpyE;
   Tag_80121C20 srcE;
   Tag_80121C20 srcB;
   Tag_80121C20 cpyB;
   Tag_80121C20 srcC;
   Tag_80121C20 cpyC;
   Tag_80121C20 srcD;
   Tag_80121C20 cpyD;
   Tag_80121C20 cpyA;
   Tag_80121C20 srcA;
   resourceListPath.flags=0; resourceListPath.length=0; resourceListPath.heap=0;
   struct StringCleanup {
     TinyStringRaw* object;
     // Release resourceListPath even when the surrounding block exits early.
     ~StringCleanup() {
       if (object->flags >> 31) appFreeUni__FPv(object->heap);
     }
   } cleanup = {&resourceListPath};
   char resourceListName[256]={0};
   if((int)AsCSharedDoc(g_sharedDoc)->partyGameMode==kPartyVariantCourseMode) {
     CSharedDoc* root=AsCSharedDoc(g_sharedDoc);
     sprintf(resourceListName,golfBackgroundStrings + kVariantListStemFormat,((SharedCourseResources*)root)->courseName,((HoleEnvironment*)root->golfDoc)->holeNumber,golfBackgroundStrings + kVariantListSuffix);
     TinyString s;
     s.flags=0; s.length=0; s.heap=0;
     int len=strlen(resourceListName); String_ReserveCapacity(&s,len);

     *(char*)&cpyA = *(volatile char*)&srcA;
    String_ReplaceRange(&s,0,0,resourceListName,((((resourceListName+len)))),&cpyA);
     int idx=((SharedCourseResources*)g_sharedDoc)->selectedWeather;
     int valid;
     if(0>idx || kWeatherResourceSuffixCount<=idx) valid=0;
     else {
       const char* p=(const char*)golfBackgroundStrings + kListSeparator;
       appendPosition=s.size();
       *(char*)&cpyB = *(volatile char*)&srcB;
       String_ReplaceRange(&s,appendPosition,0,p,p+strlen(p),&cpyB);
       p=s_courseWeatherResourceSuffixes[idx];
       appendPosition=s.size();
       *(char*)&cpyC = *(volatile char*)&srcC;
       String_ReplaceRange(&s,appendPosition,0,p,p+strlen(p),&cpyC);
       p=golfBackgroundStrings + kListZipSuffix; appendPosition=s.size();
       *(char*)&cpyD = *(volatile char*)&srcD;
       String_ReplaceRange(&s,appendPosition,0,p,p+strlen(p),&cpyD);
       valid=1;
     }
     if(!valid || !CHashPath_FindPath(s.data())) resourceListName[0]=0;
   }
   if (strlen(resourceListName)==0) {
       CSharedDoc* root=AsCSharedDoc(g_sharedDoc);
       sprintf(resourceListName,golfBackgroundStrings + kListStemFormat,((SharedCourseResources*)root)->courseName,((HoleEnvironment*)root->golfDoc)->holeNumber);
   }
   appendPosition=resourceListPath.size();
   *(char*)&cpyE = *(volatile char*)&srcE;
   int len2=strlen(resourceListName);
   String_ReplaceRange(&resourceListPath,0,appendPosition,resourceListName,resourceListName+len2,&cpyE);
   int idx=((SharedCourseResources*)g_sharedDoc)->selectedWeather;
   int valid;
   if(idx<0 || idx>=kWeatherResourceSuffixCount) valid=0;
   else {
     const char* p=golfBackgroundStrings + kListSeparator;
     appendPosition=resourceListPath.size();
     *(char*)&cpyF = *(volatile char*)&srcF;
     String_ReplaceRange(&resourceListPath,appendPosition,0,p,p+strlen(p),&cpyF);
     p=s_courseWeatherResourceSuffixes[idx]; appendPosition=resourceListPath.size();
     *(char*)&cpyG = *(volatile char*)&srcG;
     String_ReplaceRange(&resourceListPath,appendPosition,0,p,p+strlen(p),&cpyG);
     p=golfBackgroundStrings + kListZipSuffix; appendPosition=resourceListPath.size();
     *(char*)&cpyH = *(volatile char*)&srcH;
     String_ReplaceRange(&resourceListPath,appendPosition,0,p,p+strlen(p),&cpyH);
     valid=1;
   }
   if (valid) {
       resourceListArchive=CPackedFile_Create(resourceListPath.data());
       if (resourceListArchive) {
           do {
           }
           while (!CPackedFile_PollLoad(resourceListArchive));
       }
   }
 }
 // Phase 4: load terrain; rebuild sky/variant textures when weather or course changes.
 if (!CourseModel_LoadFromFile(AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->courseData,courseModelPath)) {
     g_courseLoadFailed=1;
     return;
 }
 CSharedDoc* root=AsCSharedDoc(g_sharedDoc);
 CGolfDoc* golfDocument=AsCGolfDoc((void*)root->golfDoc);
int holeIndex=golfDocument->currentHoleIndex;
 if(holeIndex == 0) environmentChanged=1;
 else {
     unsigned char* entries=((RoundCourseSelection*)root)->courseIds;
     if (*(holeIndex+entries)!=*(holeIndex+entries-1)) environmentChanged=1;
 }
 if(environmentChanged) {
   CPvsFxBox_LoadSkyResources(golfDocument->courseEffectBox,root->selectedCourseData?((IFF_STRUCT::sCourse*)root->selectedCourseData)->mapName:0,kHoleSetupZero);
   root=AsCSharedDoc(g_sharedDoc);
   CPvsFxBox_LoadVariantBitmap(AsCGolfDoc((void*)root->golfDoc)->courseEffectBox,root->selectedCourseData?((IFF_STRUCT::sCourse*)root->selectedCourseData)->mapName:0);
 }
 // Phase 5: rebuild hole objects, resume effects and publish cup/shadow placement.
 CGolfBg_InitializeCurrentHole(self);
 if(g_fx) CFx_SetUpdateEnabled(g_fx,1);
 void* object=g_taskManager ? Object_GetChild(g_taskManager) : 0;
 ((RuntimeObject*)object)->activate();
 IActor_DispatchMessage(self,self,golfBackgroundStrings + kGolfRuleActor,9,0,0,0,0);
 CGolfDoc* manager=AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
 unsigned int x;
 unsigned int y;
 HoleRecord* value=(HoleRecord*)manager->holeRecords+manager->currentHoleIndex;

 x=(unsigned int)value->cup.x;
 y=(unsigned int)value->cup.y;
 cupPositionBits.x=x; cupPositionBits.y=y; cupPositionBits.z=(unsigned int)value->cup.z;
 CupPreview_SetPosition(g_golfCupPreview,&cupPositionBits);
 if(courseId==kDefaultShadowCourseId) ShadowManager_SetCaptureQuadAlphaPreset(g_shadowManager,0,0);
 else ShadowManager_SetCaptureQuadAlphaPreset(g_shadowManager,courseId,((SharedCourseResources*)g_sharedDoc)->selectedWeather);
 if(resourceListArchive) { if(resourceListArchive) ((RuntimeObject*)resourceListArchive)->destroy(1); }
}
#pragma dont_inline on

struct CourseNamedPoint { char nameAndFlags[0x24]; Vec3f position; };
// Each entry is a 12-byte byte-string containing a sound-bank name. `resource`
// is its heap text pointer, not a sound-bank object; bit 31 owns that text.
// OnDestroy releases banks by name and destroys strings but leaves vector storage.
// `entries` and `count` must describe a valid contiguous range (possibly empty).
struct ResourceEntry { unsigned int flags; unsigned int stringLength; void* heapName; }; // heapName owns heap name text when bit 31 is set.
struct ResourceList { char precedingBackgroundFields[0x34]; ResourceEntry* entries; int count; };
struct RefObj { 
virtual void v8(int);
};

// Release the cup preview, shadow manager and loaded sound banks, then empty the owned name list.
void CGolfBg_OnDestroy(ResourceList* self) {
	if (g_golfCupPreview != 0) {
		if (g_golfCupPreview != 0) ((RefObj*)g_golfCupPreview)->v8(1);
	}
	if (g_shadowManager != 0) {
		if (g_shadowManager != 0) ((RefObj*)(void*)g_shadowManager)->v8(1);
	}
	ResourceEntry* end;
	ResourceEntry* entry = self->entries;
	int count;
	while (entry != (end = self->entries + self->count)) {
		void* name;
		if ((entry->flags >> 31) == 0) name = (char*)entry + 1;
		else name = entry->heapName;
		SoundManager_ReleaseSoundBank(g_soundManager, name);
		++entry;
	}
	count = self->count;
	self->count -= count;
	while (count != 0) {
		end = (ResourceEntry*)((char*)end - 0xc);
		if (end != 0 && (end->flags >> 31)) appFreeUni__FPv(end->heapName);
		count -= 1;
	}
}

// The returned map-name pointer is borrowed from the selected course table.
// Return the selected IFF course map name, or null when no course is selected.
unsigned int* SharedDoc_GetSelectedCourseMapName() {
	CSharedDoc* manager = AsCSharedDoc(g_sharedDoc);
	if (manager->selectedCourseData != 0) return (unsigned int*)((IFF_STRUCT::sCourse*)manager->selectedCourseData)->mapName;
	return 0;
}

// Return the selected course variant (the low nibble after Wii IFF bit repacking).
unsigned int SharedDoc_GetSelectedCourseVariant() {
	CSharedDoc* manager = AsCSharedDoc(g_sharedDoc);
	if (manager->selectedCourseData == 0) return 0;
	return ((IFF_STRUCT::sCourse*)manager->selectedCourseData)->courseFlags & 0xF;
}

// Copy the current hole's cup position as three 32-bit words.
void CGolfDoc_GetCurrentHoleCupPosition(Vector3* out) {
	CGolfDoc* manager = AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
	unsigned int x;
	unsigned int y;
	HoleRecord* value;
	value = (HoleRecord*)manager->holeRecords + manager->currentHoleIndex;
	x = (unsigned int)value->cup.x;
	y = (unsigned int)value->cup.y;
	out->x = x;
	out->y = y;
	out->z = (unsigned int)value->cup.z;
}

// Copy the current hole's tee position as three 32-bit words.
void CGolfDoc_GetCurrentHoleStartPosition(Vector3* out) {
	CGolfDoc* manager = AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
	unsigned int x;
	int y;
	HoleRecord* value;
	value = (HoleRecord*)manager->holeRecords + manager->currentHoleIndex;
	x = ((unsigned int*)manager->holeRecords)[manager->currentHoleIndex << 3];
	y = (unsigned int)value->tee.y;
	out->y = y;
	out->x = x;
	out->z = (unsigned int)value->tee.z;
}

// Lazily initialize and register the render-packet singleton.
void* RenderPacket_GetInstance() {
	if (!Sing_805DC7B0()) {
		if (s_renderPacketInitialized == 0) {
			RenderPacket_Init(s_renderPacket);
			__register_global_object(s_renderPacket, (void*)RenderPacket_Destroy, s_golfBackgroundRenderPacketDestructorRegistration);
			s_renderPacketInitialized = 1;
		}
		Sing_805DC7B0() = (void*)s_renderPacket;
	}
	return Sing_805DC7B0();
}

// Release the render packet's owned array and optionally free the packet.
void* RenderPacket_Destroy(void* self, int flags) {
	if (self != 0) {
		if (self != 0) {
			if (self != 0) {
				if (self != 0) {
					if (self != 0) {
						void* object = *(void**)self;
						if (object != 0) {
							int* count = (int*)((char*)self + 4);
							*count -= (int)(unsigned int)*count;
							appFreeUni__FPv(object);
						}
					}
				}
			}
		}
		if (flags > 0) __dl__FPv(self);
	}
	return self;
}

// Return the shadow manager used for the hole's sun setup.
unsigned int ShadowManager_GetCurrent() {
	return g_shadowManager;
}



// Return the zero-based hole index within the configured round.
unsigned char CGolfDoc_GetCurrentHoleIndex(CGolfDoc* arg) {
	return arg->currentHoleIndex;
}

// Return the course file's hole number used in resource paths.
unsigned char CGolfDoc_GetCurrentHoleNumber(CGolfDoc* arg) {
	return ((HoleEnvironment*)arg)->holeNumber;
}

// Return the task manager's current child, or null without a task manager.
void* TaskManager_GetCurrentChild() {
	void* p = g_taskManager;
	if (p == 0) {
		return 0;
	}
	return Object_GetChild(p);
}



// Requires a valid currentHoleIndex; the result borrows document-owned storage.
// Return the current 0x20-byte tee/cup/par hole record.
void* Manager_GetCurrentEntry(CGolfDoc* arg) {
	return (HoleRecord*)arg->holeRecords + arg->currentHoleIndex;
}

struct fn_80122794_Arg {
	char precedingPacketFields[0x4C];
    char shadowData[1];
};

// Return the render packet's embedded shadow-data block.
void* RenderPacket_GetShadowData(fn_80122794_Arg* arg) {
	return arg->shadowData;
}

struct fn_801228D8_Arg {
	char precedingBallFields[0xC0];
	float radius;
};

// Return the physical radius used to lift the ball above the tee ground.
float CGolfBall_GetRadius(fn_801228D8_Arg* arg) {
	return arg->radius;
}

struct fn_801228E0_Arg {
	char precedingEntryFields[0x10];
	unsigned int kind;
};

// Return the course entry kind used to choose scene-object construction.
unsigned int CourseEntry_GetKind(fn_801228E0_Arg* arg) {
	return arg->kind;
}

// Set all three components and return the output vector.
Vec3f* Vec3_Set(Vec3f* arg, float a, float b, float c) {
	arg->x = a;
	arg->y = b;
	arg->z = c;
	return arg;
}

#pragma dont_inline off

// Reset the hole, place tee/cup/ball and course objects, then calculate each match player's dormie state.
void CGolfBg_InitializeCurrentHole(void* background) {
	Vector3 ballPosition;
	Vector3 raisedTeePosition;
	Vector3 teePosition;
	Vec3f ballRadiusOffset;
	Vector3 cupPosition;
	void* courseEntryIterator;
	void* iteratorCopy;
	void* firstCourseEntry;
	void* endCourseEntry;

	void* golfDocument = (void*)GolfDoc_GetCurrent();
	// Phase 1: restore player state and ground the ambient, tee and cup positions.
	CGolfDoc_RestoreFirstPlayerSnapshot(golfDocument);
	CourseData_BuildQuadTree(AsCGolfDoc(GolfDoc_GetCurrent())->courseData);
	GolfBg_DispatchCameraMessage2(background);
	if ((unsigned char)SharedDoc_GetSelectedCourseVariant() == 1) {
		void* t = CourseData_FindNamedPoint(AsCGolfDoc(GolfDoc_GetCurrent())->courseData, golfBackgroundStrings + kPrimaryGridPoint);
		CGolfBg_GroundAndDispatchAmbientPosition(background, &((CourseNamedPoint*)t)->position);
	} else {
		void* t = CourseData_FindNamedPoint(AsCGolfDoc(GolfDoc_GetCurrent())->courseData, golfBackgroundStrings + kAlternateGridPoint);
		if (t == 0) {
			t = CourseData_FindNamedPoint(AsCGolfDoc(GolfDoc_GetCurrent())->courseData, golfBackgroundStrings + kPrimaryGridPoint);
		}
		CGolfBg_GroundAndDispatchAmbientPosition(background, &((CourseNamedPoint*)t)->position);
	}
	CGolfDoc_SetCurrentHoleStartOnGround(background, Manager_GetCurrentEntry((CGolfDoc*)GolfDoc_GetCurrent()));
	CGolfDoc_SetCurrentHoleCupOnGround(background, &((HoleRecord*)Manager_GetCurrentEntry((CGolfDoc*)GolfDoc_GetCurrent()))->cup);
	// Phase 2: rebuild course effects and copy the named sun settings into the packet.
	if (AsCGolfDoc(GolfDoc_GetCurrent())->courseEffectBox != 0) {
		CPvsFxBox_ReloadCourseEffects(AsCGolfDoc(GolfDoc_GetCurrent())->courseEffectBox);
		CPvsFxBox_RebuildAuxiliaryEffect(AsCGolfDoc(GolfDoc_GetCurrent())->courseEffectBox);
		CPvsFxBox_RebuildSceneEffects(AsCGolfDoc(GolfDoc_GetCurrent())->courseEffectBox);
	}
	{
		void* r = RenderPacket_GetShadowData((fn_80122794_Arg*)RenderPacket_GetInstance());
		CShadowManager_CopyNamedRecordSettings((void*)ShadowManager_GetCurrent(), golfBackgroundStrings + kSunName, r);
	}
	// World Y is up: place the ball at tee + (0, physical radius, 0).
	if (GolfTask_NoOpStatus() == 0) {
		Vec3f* p;
		float f2 = CGolfBall_GetRadius((fn_801228D8_Arg*)GolfBall__Fv());
		p = Vec3_Set(&ballRadiusOffset, kHoleSetupZero, f2, kHoleSetupZero);
		CGolfDoc_GetCurrentHoleStartPosition(&teePosition);
		__pl__FRC7WVectorRC7WVector(&raisedTeePosition, &teePosition, p);
		ballPosition = raisedTeePosition;
		CGolfBall_Reset(GolfBall__Fv(), &ballPosition);
	}
	// Phase 3: create scene objects, then rebuild aim paths from kind-4 course entries.
	void* sceneManager = (void*)SceneManager_GetCurrent();
	CGolfBg_CreateCourseObjects(sceneManager);
	GolfAimPath_Clear(AsCGolfDoc(GolfDoc_GetCurrent())->aimPath);
	CourseEntryIterator_DefaultConstruct(&courseEntryIterator);
	firstCourseEntry = CourseEntryVector_Begin(&AsCourseData801C(AsCGolfDoc(GolfDoc_GetCurrent())->courseData)->regions);
	CourseEntryIterator_Assign(&courseEntryIterator, CourseEntryIterator_CopyConstruct(&iteratorCopy, &firstCourseEntry, 0));
	while ((endCourseEntry = CourseEntryVector_End(&AsCourseData801C(AsCGolfDoc(GolfDoc_GetCurrent())->courseData)->regions), CourseEntryIterator_NotEqual(&courseEntryIterator, &endCourseEntry))) {
		if ((int)CourseEntry_GetKind((fn_801228E0_Arg*)CourseEntryIterator_GetEntry(&courseEntryIterator)) == kAimPathEntryKind) {
			void* t;
			CGolfDoc_GetCurrentHoleCupPosition(&cupPosition);
			void* courseEntry = (void*)CourseEntryIterator_GetEntry(&courseEntryIterator);
			t = CourseEntry_GetPointVector(courseEntry);
			Vec3Vector_CopyAppendAndFlattenHeight(AsCGolfDoc(GolfDoc_GetCurrent())->aimPath, t, &cupPosition);
		}
		CourseEntryIterator_PostIncrement(&courseEntryIterator, 0);
	}
	// Flattening has copied the point data; release the course's temporary resources.
	CourseData_ReleaseResources(AsCGolfDoc(GolfDoc_GetCurrent())->courseData, 1);
	{
		// Phase 4: match-play opponents are paired by index ^ 1. Compare the
		// lead to holes remaining including this hole; skip the final-hole special case.
		int playerIndex = 0;
		int noMatchPoint = kNoMatchPoint;
		int dormie = kDormie;
		int oneShortOfDormie = kOneShortOfDormie;
		for (; (playerIndex < CGolfDoc_GetPlayerCount(GolfDoc_GetCurrent())); playerIndex++) {
			void* playerRecord = GolfPlayer_GetCurrentEntryByIndex((unsigned char)playerIndex);
			((MatchPlayerRecord*)playerRecord)->matchPointState = noMatchPoint;
			if (SharedDoc_GetGolfRuleType() == kMatchPlay) {
				if ((unsigned int)CGolfDoc_GetCurrentHoleIndex((CGolfDoc*)GolfDoc_GetCurrent()) != (unsigned int)SharedDoc_GetLastRoundHoleIndex()) {
					unsigned int roundHoleCount = SharedDoc_GetRoundHoleCount();
					int holesStillToPlay = roundHoleCount - CGolfDoc_GetCurrentHoleIndex((CGolfDoc*)GolfDoc_GetCurrent());
					int holesAhead = ((MatchPlayerRecord*)GolfPlayer_GetCurrentEntryByIndex((unsigned char)playerIndex))->holesWon
					      - ((MatchPlayerRecord*)GolfPlayer_GetCurrentEntryByIndex((unsigned char)(playerIndex ^ 1)))->holesWon;
					if (holesStillToPlay == holesAhead) {
						((MatchPlayerRecord*)GolfPlayer_GetCurrentEntryByIndex((unsigned char)playerIndex))->matchPointState = dormie;
					} else if (holesStillToPlay == holesAhead + 1) {
						((MatchPlayerRecord*)GolfPlayer_GetCurrentEntryByIndex((unsigned char)playerIndex))->matchPointState = oneShortOfDormie;
					}
				}
			}
		}
	}
}

} // extern "C"
