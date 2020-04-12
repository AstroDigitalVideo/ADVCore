/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADV_LIB
#define ADV_LIB

#include "adv_file.h"
#include "adv2_file.h"
#include "adv2_status_section.h"

extern char* g_CurrentAdvFile;
extern AdvLib::AdvFile* g_AdvFile;
extern AdvLib2::Adv2File* g_Adv2File;
extern bool g_FileStarted;

#define CORE_VERSION "2.1d"

#ifdef __cplusplus
extern "C"
{
#endif 

DLL_PUBLIC char* AdvGetCurrentFilePath(void);
DLL_PUBLIC unsigned int AdvGetFileVersion(const char* fileName);
DLL_PUBLIC int AdvOpenFile(const char* fileName, AdvLib2::AdvFileInfo* fileInfo);
DLL_PUBLIC unsigned int AdvCloseFile();

DLL_PUBLIC void AdvVer1_NewFile(const char* fileName);
DLL_PUBLIC void AdvVer1_DefineImageSection(unsigned short width, unsigned short height, unsigned char dataBpp);
DLL_PUBLIC void AdvVer1_DefineImageLayout(unsigned char layoutId, const char* layoutType, const char* compression, unsigned char layoutBpp, int keyFrame, const char* diffCorrFromBaseFrame);
DLL_PUBLIC unsigned int AdvVer1_DefineStatusSectionTag(const char* tagName, int tagType);
DLL_PUBLIC unsigned int AdvVer1_AddFileTag(const char* tagName, const char* tagValue);
DLL_PUBLIC void AdvVer1_AddOrUpdateImageSectionTag(const char* tagName, const char* tagValue);
DLL_PUBLIC void AdvVer1_EndFile();
DLL_PUBLIC bool AdvVer1_BeginFrame(__int64 timeStamp, unsigned int elapsedTime, unsigned int exposure);
DLL_PUBLIC void AdvVer1_FrameAddImage(unsigned char layoutId, unsigned short* pixels, unsigned char pixelsBpp);
DLL_PUBLIC void AdvVer1_FrameAddImageBytes(unsigned char layoutId, unsigned char* pixels, unsigned char pixelsBpp);
DLL_PUBLIC void AdvVer1_FrameAddStatusTag(unsigned int tagIndex, const char* tagValue);
DLL_PUBLIC void AdvVer1_FrameAddStatusTagMessage(unsigned int tagIndex, const char* tagValue);
DLL_PUBLIC void AdvVer1_FrameAddStatusTagUInt8(unsigned int tagIndex, unsigned char tagValue);
DLL_PUBLIC void AdvVer1_FrameAddStatusTag16(unsigned int tagIndex, unsigned short tagValue);
DLL_PUBLIC void AdvVer1_FrameAddStatusTagReal(unsigned int tagIndex, float tagValue);
DLL_PUBLIC void AdvVer1_FrameAddStatusTag32(unsigned int tagIndex, unsigned int tagValue);
DLL_PUBLIC void AdvVer1_FrameAddStatusTag64(unsigned int tagIndex, __int64 tagValue);
DLL_PUBLIC void AdvVer1_EndFrame();

DLL_PUBLIC ADVRESULT AdvVer2_NewFile(const char* fileName, bool overwriteExisting);
DLL_PUBLIC ADVRESULT AdvVer2_SetTicksTimingPrecision(int mainStreamAccuracy, int calibrationStreamAccuracy);
DLL_PUBLIC ADVRESULT AdvVer2_DefineExternalClockForMainStream(__int64 clockFrequency, int ticksTimingAccuracy);
DLL_PUBLIC ADVRESULT AdvVer2_DefineExternalClockForCalibrationStream(__int64 clockFrequency, int ticksTimingAccuracy);
DLL_PUBLIC ADVRESULT AdvVer2_AddMainStreamTag(const char* tagName, const char* tagValue);
DLL_PUBLIC ADVRESULT AdvVer2_AddCalibrationStreamTag(const char* tagName, const char* tagValue);
DLL_PUBLIC ADVRESULT AdvVer2_DefineImageSection(unsigned short width, unsigned short height, unsigned char dataBpp);
DLL_PUBLIC ADVRESULT AdvVer2_DefineImageLayout(unsigned char layoutId, const char* layoutType, const char* compression, unsigned char layoutBpp);
DLL_PUBLIC ADVRESULT AdvVer2_DefineStatusSection(__int64 utcTimestampAccuracyInNanoseconds);
DLL_PUBLIC ADVRESULT AdvVer2_DefineStatusSectionTag(const char* tagName, int tagType, unsigned int* addedTagId);
DLL_PUBLIC ADVRESULT AdvVer2_AddFileTag(const char* tagName, const char* tagValue);
DLL_PUBLIC ADVRESULT AdvVer2_AddUserTag(const char* tagName, const char* tagValue);
DLL_PUBLIC ADVRESULT AdvVer2_AddOrUpdateImageSectionTag(const char* tagName, const char* tagValue);
DLL_PUBLIC ADVRESULT AdvVer2_EndFile();
DLL_PUBLIC ADVRESULT AdvVer2_BeginFrameWithTicks(unsigned int streamId, __int64 startFrameTicks, __int64 endFrameTicks, __int64 elapsedTicksSinceFirstFrame, __int64 utcStartTimeNanosecondsSinceAdvZeroEpoch, unsigned int utcExposureNanoseconds);
DLL_PUBLIC ADVRESULT AdvVer2_BeginFrame(unsigned int streamId, __int64 utcStartTimeNanosecondsSinceAdvZeroEpoch, unsigned int utcExposureNanoseconds);
DLL_PUBLIC ADVRESULT AdvVer2_FrameAddImage(unsigned char layoutId, unsigned short* pixels, unsigned char pixelsBpp);
DLL_PUBLIC ADVRESULT AdvVer2_FrameAddImageBytes(unsigned char layoutId, unsigned char* pixels, unsigned char pixelsBpp);
DLL_PUBLIC ADVRESULT AdvVer2_FrameAddStatusTagUTF8String(unsigned int tagIndex, const char* tagValue);
DLL_PUBLIC ADVRESULT AdvVer2_FrameAddStatusTagUInt8(unsigned int tagIndex, unsigned char tagValue);
DLL_PUBLIC ADVRESULT AdvVer2_FrameAddStatusTag16(unsigned int tagIndex, unsigned short tagValue);
DLL_PUBLIC ADVRESULT AdvVer2_FrameAddStatusTagReal(unsigned int tagIndex, float tagValue);
DLL_PUBLIC ADVRESULT AdvVer2_FrameAddStatusTag32(unsigned int tagIndex, unsigned int tagValue);
DLL_PUBLIC ADVRESULT AdvVer2_FrameAddStatusTag64(unsigned int tagIndex, __int64 tagValue);
DLL_PUBLIC ADVRESULT AdvVer2_EndFrame();

DLL_PUBLIC ADVRESULT AdvVer2_GetFramePixels(int streamId, int frameNo, unsigned int* pixels, AdvLib2::AdvFrameInfo* frameInfo, int* systemErrorLen);
DLL_PUBLIC ADVRESULT AdvVer2_GetStackedFramePixels(int streamId, int frameNo, int framesToStack, bool isSlidingIntegration, unsigned int* pixels, AdvLib2::AdvFrameInfo* frameInfo);
DLL_PUBLIC ADVRESULT AdvVer2_GetTagPairSizes(TagPairType tagPairType, int tagId, int* tagNameSize, int* tagValueSize);
DLL_PUBLIC ADVRESULT AdvVer2_GetTagPairValues(TagPairType tagPairType, int tagId, char* tagName, char* tagValue);
DLL_PUBLIC ADVRESULT AdvVer2_GetImageLayoutInfo(int layoutIndex, AdvLib2::AdvImageLayoutInfo* imageLayoutInfo);
DLL_PUBLIC ADVRESULT AdvVer2_GetStatusTagNameSize(unsigned int tagId, int* tagNameSize);
DLL_PUBLIC ADVRESULT AdvVer2_GetStatusTagInfo(unsigned int tagId, char* tagName, Adv2TagType* tagType);
DLL_PUBLIC ADVRESULT AdvVer2_GetStatusTagSizeUTF8String(unsigned int tagIndex, int* tagValueSize);
DLL_PUBLIC ADVRESULT AdvVer2_GetStatusTagUTF8String(unsigned int tagIndex, char* tagValue);
DLL_PUBLIC ADVRESULT AdvVer2_GetStatusTagUInt8(unsigned int tagIndex, unsigned char* tagValue);
DLL_PUBLIC ADVRESULT AdvVer2_GetStatusTag16(unsigned int tagIndex, unsigned short* tagValue);
DLL_PUBLIC ADVRESULT AdvVer2_GetStatusTagReal(unsigned int tagIndex, float* tagValue);
DLL_PUBLIC ADVRESULT AdvVer2_GetStatusTag32(unsigned int tagIndex, unsigned int* tagValue);
DLL_PUBLIC ADVRESULT AdvVer2_GetStatusTag64(unsigned int tagIndex, __int64* tagValue);
DLL_PUBLIC ADVRESULT AdvVer2_GetIndexEntries(AdvLib2::AdvIndexEntry* mainIndex, AdvLib2::AdvIndexEntry* calibrationIndex);

DLL_PUBLIC int AdvVer2_GetLastSystemSpecificFileError();

DLL_PUBLIC void GetLibraryVersion(char* version);
DLL_PUBLIC void GetLibraryPlatformId( char* platform);
DLL_PUBLIC int GetLibraryBitness();


#ifdef __cplusplus
}
#endif
 
#endif

