/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "stdafx.h"
#include <stdio.h>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <string.h>
#include <cstdint>

#include "adv_lib.h"
#include "adv_image_layout.h"
#include "adv_profiling.h"
#include "adv2_file.h"
#include "adv2_error_codes.h"


char* g_CurrentAdvFile;
AdvLib::AdvFile* g_AdvFile;
bool g_FileStarted = false;

AdvLib2::Adv2File* g_Adv2File;

using namespace std;

char* AdvGetCurrentFilePath(void)
{
	return g_CurrentAdvFile;
}

unsigned int AdvGetFileVersion(const char* fileName)
{
	FILE* probe = advfopen(fileName, "rb");
	if (probe == 0) return 0;
	
	unsigned int buffInt;
	unsigned char buffChar;

	advfread(&buffInt, 4, 1, probe);
	advfread(&buffChar, 1, 1, probe);
	advfclose(probe);	
	
	if (buffInt != 0x46545346) return 0;
	return (unsigned int)buffChar;
}

int AdvOpenFile(const char* fileName, AdvLib2::AdvFileInfo* fileInfo)
{
	AdvCloseFile();

	FILE* probe = advfopen(fileName, "rb");
	if (probe == 0) return E_ADV_IO_ERROR;
	
	unsigned int buffInt;
	unsigned char buffChar;

	advfread(&buffInt, 4, 1, probe);
	advfread(&buffChar, 1, 1, probe);
	advfclose(probe);
	
	if (buffInt != 0x46545346) return E_ADV_NOT_AN_ADV_FILE;
	
	if (buffChar == 1)
	{
		if (nullptr != g_AdvFile)
		{
			delete g_AdvFile;
			g_AdvFile = nullptr;
		}
		
		g_FileStarted = false;
		
		int len = (int)strlen(fileName);
		if (len > 0)
		{
			g_CurrentAdvFile = new char[len + 1];
			strncpy_s(g_CurrentAdvFile, len + 1, fileName, len + 1);
		
			g_AdvFile = new AdvLib::AdvFile();
			int res = g_AdvFile->LoadFile(fileName);
			if (res < 0)
			{
				delete g_AdvFile;
				g_AdvFile = nullptr;
				return res;
			}
		}
		
		return 1;
	}
	else if (buffChar == 2)
	{
		if (nullptr != g_Adv2File)
		{
			delete g_Adv2File;
			g_Adv2File = nullptr;
		}
		
		g_FileStarted = false;
		
		int len = (int)strlen(fileName);
		if (len > 0)
		{
			g_CurrentAdvFile = new char[len + 1];
			strncpy_s(g_CurrentAdvFile, len + 1, fileName, len + 1);
		
			g_Adv2File = new AdvLib2::Adv2File();
			int res = g_Adv2File->LoadFile(fileName, fileInfo);
			if (res < 0)
			{
				delete g_Adv2File;
				g_Adv2File = nullptr;
				return res;
			}
		}
		
		return 2;
	}

	return E_ADV_VERSION_NOT_SUPPORTED;
}

unsigned int AdvCloseFile()
{
	unsigned int rv = 0;

	if (nullptr != g_AdvFile)
	{
		g_AdvFile->CloseFile();
		delete g_AdvFile;
		g_AdvFile = nullptr;
		rv += 1;
	}

	if (nullptr != g_Adv2File)
	{
		g_Adv2File->CloseFile();
		delete g_Adv2File;
		g_Adv2File = nullptr;
		rv += 2;
	}

	if (nullptr != g_CurrentAdvFile)
	{
		delete g_CurrentAdvFile;
		g_CurrentAdvFile = nullptr;
	}

	return rv;
}

void AdvVer1_NewFile(const char* fileName)
{
	AdvProfiling_ResetPerformanceCounters();
	AdvProfiling_StartProcessing();
	
    if (nullptr != g_AdvFile)
	{
		delete g_AdvFile;
		g_AdvFile = nullptr;		
	}
	
	if (nullptr != g_CurrentAdvFile)
	{
		delete g_CurrentAdvFile;
		g_CurrentAdvFile = nullptr;
	}
	
	g_FileStarted = false;
	
	int len = (int)strlen(fileName);	
	if (len > 0)
	{
		g_CurrentAdvFile = new char[len + 1];
		strncpy_s(g_CurrentAdvFile, len + 1, fileName, len + 1);
	
		g_AdvFile = new AdvLib::AdvFile();	
	}
	AdvProfiling_EndProcessing();
}

void AdvVer1_EndFile()
{
	if (nullptr != g_AdvFile)
	{
		g_AdvFile->EndFile();
		
		delete g_AdvFile;
		g_AdvFile = nullptr;		
	}
	
	if (nullptr != g_CurrentAdvFile)
	{
		delete g_CurrentAdvFile;
		g_CurrentAdvFile = nullptr;
	}
	
	g_FileStarted = false;
}

void AdvVer1_DefineImageSection(unsigned short width, unsigned short height, unsigned char dataBpp)
{
	AdvProfiling_StartProcessing();
	AdvLib::AdvImageSection* imageSection = new AdvLib::AdvImageSection(width, height, dataBpp);
	g_AdvFile->AddImageSection(imageSection);
	AdvProfiling_EndProcessing();
}

void AdvVer1_DefineImageLayout(unsigned char layoutId, const char* layoutType, const char* compression, unsigned char bpp, int keyFrame, const char* diffCorrFromBaseFrame)
{	
	AdvProfiling_StartProcessing();
	AdvLib::AdvImageLayout* imageLayout = g_AdvFile->ImageSection->AddImageLayout(layoutId, layoutType, compression, bpp, keyFrame);
	if (diffCorrFromBaseFrame != nullptr)
		imageLayout->AddOrUpdateTag("DIFFCODE-BASE-FRAME", diffCorrFromBaseFrame);
		
	AdvProfiling_EndProcessing();
}

unsigned int AdvVer1_DefineStatusSectionTag(const char* tagName, int tagType)
{
	AdvProfiling_StartProcessing();
	unsigned int statusTagId = g_AdvFile->StatusSection->DefineTag(tagName, (AdvTagType)tagType);
	AdvProfiling_EndProcessing();
	return statusTagId;
}

unsigned int AdvVer1_AddFileTag(const char* tagName, const char* tagValue)
{
	AdvProfiling_StartProcessing();	
	unsigned int fileTagId = g_AdvFile->AddFileTag(tagName, tagValue);
	AdvProfiling_EndProcessing();
	return fileTagId;
}

void AdvVer1_AddOrUpdateImageSectionTag(const char* tagName, const char* tagValue)
{
	AdvProfiling_StartProcessing();
	return g_AdvFile->ImageSection->AddOrUpdateTag(tagName, tagValue);
	AdvProfiling_EndProcessing();
}

bool AdvVer1_BeginFrame(__int64 timeStamp, unsigned int elapsedTime, unsigned int exposure)
{
	AdvProfiling_StartProcessing();
	if (!g_FileStarted)
	{
		bool success = g_AdvFile->BeginFile(g_CurrentAdvFile);
		if (success)
		{
			g_FileStarted = true;	
		}
		else
		{
			g_FileStarted = false;
			return false;
		}		
	}
	
	g_AdvFile->BeginFrame(timeStamp, elapsedTime, exposure);
	AdvProfiling_EndProcessing();
	return true;
}

void AdvVer1_FrameAddImageBytes(unsigned char layoutId,  unsigned char* pixels, unsigned char pixelsBpp)
{
	AdvProfiling_StartProcessing();
	g_AdvFile->AddFrameImage(layoutId, (unsigned short*)pixels, pixelsBpp);
	AdvProfiling_EndProcessing();
}

void AdvVer1_FrameAddImage(unsigned char layoutId,  unsigned short* pixels, unsigned char pixelsBpp)
{
	AdvProfiling_StartProcessing();
	g_AdvFile->AddFrameImage(layoutId, pixels, pixelsBpp);
	AdvProfiling_EndProcessing();
}

void AdvVer1_FrameAddStatusTag(unsigned int tagIndex, const char* tagValue)
{
	AdvProfiling_StartProcessing();
	g_AdvFile->AddFrameStatusTag(tagIndex, tagValue);
	AdvProfiling_EndProcessing();
}

void AdvVer1_FrameAddStatusTagMessage(unsigned int tagIndex, const char* tagValue)
{	
	AdvProfiling_StartProcessing();
	g_AdvFile->AddFrameStatusTagMessage(tagIndex, tagValue);
	AdvProfiling_EndProcessing();
}

void AdvVer1_FrameAddStatusTagUInt8(unsigned int tagIndex, unsigned char tagValue)
{
	AdvProfiling_StartProcessing();
	g_AdvFile->AddFrameStatusTagUInt8(tagIndex, tagValue);
	AdvProfiling_EndProcessing();	
}

void AdvVer1_FrameAddStatusTag32(unsigned int tagIndex, unsigned int tagValue)
{
	AdvProfiling_StartProcessing();
	g_AdvFile->AddFrameStatusTagUInt32(tagIndex, tagValue);
	AdvProfiling_EndProcessing();
}

void AdvVer1_FrameAddStatusTag64(unsigned int tagIndex, __int64 tagValue)
{
	AdvProfiling_StartProcessing();
	g_AdvFile->AddFrameStatusTagUInt64(tagIndex, tagValue);
	AdvProfiling_EndProcessing();
}

void AdvVer1_FrameAddStatusTag16(unsigned int tagIndex, unsigned short tagValue)
{
	AdvProfiling_StartProcessing();
	g_AdvFile->AddFrameStatusTagUInt16(tagIndex, tagValue);
	AdvProfiling_EndProcessing();	
}

void AdvVer1_FrameAddStatusTagReal(unsigned int tagIndex, float tagValue)
{
	AdvProfiling_StartProcessing();
	g_AdvFile->AddFrameStatusTagReal(tagIndex, tagValue);
	AdvProfiling_EndProcessing();	
}

void AdvVer1_EndFrame()
{
	AdvProfiling_StartProcessing();
	g_AdvFile->EndFrame();
	AdvProfiling_EndProcessing();
}

void GetLibraryVersion(char* version)
{
	strcpy_s(version, strlen(CORE_VERSION) + 1, CORE_VERSION);
}

int GetLibraryBitness()
{
	#if __GNUC__
	
		#if defined(_WIN64)
			// Windows compilation with GCC
			return 64;
		#elif defined(_WIN32)
			// Windows compilation with GCC
			return 32;
		#endif
		
		// Linux/OSX Compilation
		
		// All modern 64-bit Unix systems use LP64. MacOS X and Linux are both modern 64-bit systems.
		//	Type           ILP64   LP64   LLP64
		//  char              8      8       8
		//  short            16     16      16
		//  int              64     32      32
		//  long             64     64      32
		//  long long        64     64      64
		//  pointer          64     64      64
		//------------------------------------
		// On a Unix system (gcc/g++ compiler) the bitness can be determined by the size of 'long'
		return sizeof(long) * 8;
		
	#endif
	#if _WIN32 || _WIN64

		#if defined(_WIN64)
			return 64;  // 64-bit programs run only on Win64
			
		#elif defined(_WIN32)
			//// 32-bit programs run on both 32-bit and 64-bit Windows so must sniff
			//BOOL f64 = FALSE;
			//if (IsWow64Process(GetCurrentProcess(), &f64) && f64)
			//	return 64;
			//else
			//	return 32;

			// We only care if the binary is 32 or 64 bit, so ignore the IsWow64Process thing
			return 32;
		#else
			return 16; // Win64 does not support Win16
		#endif
	#endif
}

void GetLibraryPlatformId(char* platform)
{
#define PLATFORM_WIN_MSVC_32 "MS VC++, x86, Windows"
#define PLATFORM_WIN_MSVC_64 "MS VC++, AMD64, Windows"
#define PLATFORM_WIN_GNU_32 "GNU GCC/G++, x86, Windows"
#define PLATFORM_WIN_GNU_64 "GNU GCC/G++, AMD64, Windows"
#define PLATFORM_LINUX_GNU "GNU GCC/G++, Linux"
#define PLATFORM_OSX_GNU "GNU GCC/G++, OSX"
#define PLATFORM_UNKNOWN "Unknown"

#ifdef MSVC
	#if INTPTR_MAX == INT32_MAX
		strcpy_s(platform, strlen(PLATFORM_WIN_MSVC_32) + 1, PLATFORM_WIN_MSVC_32);
	#elif INTPTR_MAX == INT64_MAX
		strcpy_s(platform, strlen(PLATFORM_WIN_MSVC_64) + 1, PLATFORM_WIN_MSVC_64);
	#endif
#elif __linux__
	strcpy_s(platform, strlen(PLATFORM_LINUX_GNU) + 1, PLATFORM_LINUX_GNU);
#elif __APPLE__
	strcpy_s(platform, strlen(PLATFORM_OSX_GNU) + 1, PLATFORM_OSX_GNU);
#elif __GNUC__ || __GNUG__
	#if __x86_64__ || __ppc64__ || _WIN64
		strcpy_s(platform, strlen(PLATFORM_WIN_GNU_64) + 1, PLATFORM_WIN_GNU_64);
	#else
		strcpy_s(platform, strlen(PLATFORM_WIN_GNU_32) + 1, PLATFORM_WIN_GNU_32);
	#endif	
#else
	strcpy_s(platform, strlen(PLATFORM_UNKNOWN) + 1, PLATFORM_UNKNOWN);
#endif
}

ADVRESULT AdvVer2_NewFile(const char* fileName, bool overwriteExisting)
{
	if (fileName == nullptr)
		return E_FAIL;

	AdvProfiling_ResetPerformanceCounters();
	AdvProfiling_StartProcessing();
	
    if (nullptr != g_Adv2File)
	{
		delete g_Adv2File;
		g_Adv2File = nullptr;
	}
	
	if (nullptr != g_CurrentAdvFile)
	{
		delete g_CurrentAdvFile;
		g_CurrentAdvFile = nullptr;
	}
	
	g_FileStarted = false;
	
	ADVRESULT rv = CheckFileName(fileName, !overwriteExisting);
	if (rv != S_OK)
		return rv;

	int len = (int)strlen(fileName);
	if (len > 0)
	{
		g_CurrentAdvFile = new char[len + 1];
		strncpy_s(g_CurrentAdvFile, len + 1, fileName, len + 1);
		
		g_Adv2File = new AdvLib2::Adv2File();
	}
	AdvProfiling_EndProcessing();

	return S_OK;
}

ADVRESULT AdvVer2_SetTicksTimingPrecision(int mainStreamAccuracy, int calibrationStreamAccuracy)
{
	if (g_Adv2File == nullptr)
		return E_ADV_NOFILE;

	return g_Adv2File->SetTicksTimingPrecision(mainStreamAccuracy, calibrationStreamAccuracy);
}

ADVRESULT AdvVer2_DefineExternalClockForMainStream(__int64 clockFrequency, int ticksTimingAccuracy)
{
	if (g_Adv2File == nullptr)
		return E_ADV_NOFILE;

	return g_Adv2File->DefineExternalClockForMainStream(clockFrequency, ticksTimingAccuracy);
}

ADVRESULT AdvVer2_DefineExternalClockForCalibrationStream(__int64 clockFrequency, int ticksTimingAccuracy)
{
	if (g_Adv2File == nullptr)
		return E_ADV_NOFILE;

	return g_Adv2File->DefineExternalClockForCalibrationStream(clockFrequency, ticksTimingAccuracy);
}


ADVRESULT AdvVer2_EndFile()
{
	ADVRESULT rv = S_OK;

	if (nullptr != g_Adv2File)
	{
		rv = g_Adv2File->EndFile();
		
		delete g_Adv2File;
		g_Adv2File = nullptr;		
	}
	else
		rv = E_ADV_NOFILE;
	
	if (nullptr != g_CurrentAdvFile)
	{
		delete g_CurrentAdvFile;
		g_CurrentAdvFile = nullptr;
	}
	
	g_FileStarted = false;
	return rv;
}

ADVRESULT AdvVer2_AddMainStreamTag(const char* tagName, const char* tagValue)
{
	if (g_Adv2File == nullptr)
		return E_ADV_NOFILE;

	AdvProfiling_StartProcessing();
	ADVRESULT rv = g_Adv2File->AddMainStreamTag(tagName, tagValue);
	AdvProfiling_EndProcessing();

	return rv;
}

ADVRESULT AdvVer2_AddCalibrationStreamTag(const char* tagName, const char* tagValue)
{
	if (g_Adv2File == nullptr)
		return E_ADV_NOFILE;

	AdvProfiling_StartProcessing();
	ADVRESULT rv = g_Adv2File->AddCalibrationStreamTag(tagName, tagValue);
	AdvProfiling_EndProcessing();

	return rv;
}

ADVRESULT AdvVer2_BeginFrame(unsigned int streamId, __int64 utcStartTimeNanosecondsSinceAdvZeroEpoch, unsigned int utcExposureNanoseconds)
{
	ADVRESULT rv = S_OK;
	AdvProfiling_StartProcessing();
	if (!g_FileStarted)
	{
		rv = g_Adv2File->BeginFile(g_CurrentAdvFile);
		if (rv == S_OK)
		{
			g_FileStarted = true;
		}
		else
		{
			g_FileStarted = false;
			return rv;
		}		
	}
	
	rv = g_Adv2File->BeginFrame(streamId, utcStartTimeNanosecondsSinceAdvZeroEpoch, utcExposureNanoseconds);
	AdvProfiling_EndProcessing();
	return rv;
}

ADVRESULT AdvVer2_BeginFrameWithTicks(unsigned int streamId, __int64 startFrameTicks, __int64 endFrameTicks, __int64 elapsedTicksSinceFirstFrame, __int64 utcStartTimeNanosecondsSinceAdvZeroEpoch, unsigned int utcExposureNanoseconds)
{
	ADVRESULT rv = S_OK;
	AdvProfiling_StartProcessing();
	if (!g_FileStarted)
	{
		rv = g_Adv2File->BeginFile(g_CurrentAdvFile);
		if (rv == S_OK)
		{
			g_FileStarted = true;	
		}
		else
		{
			g_FileStarted = false;
			return rv;
		}		
	}
	
	rv = g_Adv2File->BeginFrame(streamId, startFrameTicks, endFrameTicks, elapsedTicksSinceFirstFrame, utcStartTimeNanosecondsSinceAdvZeroEpoch, utcExposureNanoseconds);
	AdvProfiling_EndProcessing();
	return rv;
}

ADVRESULT AdvVer2_EndFrame()
{
	if (g_Adv2File == nullptr)
		return E_ADV_NOFILE;

	AdvProfiling_StartProcessing();
	ADVRESULT rv = g_Adv2File->EndFrame();
	AdvProfiling_EndProcessing();

	return rv;
}

ADVRESULT AdvVer2_DefineImageSection(unsigned short width, unsigned short height, unsigned char dataBpp)
{
	if (g_Adv2File == nullptr)
		return E_ADV_NOFILE;

	AdvProfiling_StartProcessing();
	AdvLib2::Adv2ImageSection* imageSection = new AdvLib2::Adv2ImageSection(width, height, dataBpp);
	ADVRESULT rv = g_Adv2File->AddImageSection(imageSection);
	AdvProfiling_EndProcessing();
	return rv;
}

ADVRESULT AdvVer2_DefineStatusSection(__int64 utcTimestampAccuracyInNanoseconds)
{
	if (g_Adv2File == nullptr)
		return E_ADV_NOFILE;

	AdvProfiling_StartProcessing();
	AdvLib2::Adv2StatusSection* statusSection = new AdvLib2::Adv2StatusSection(utcTimestampAccuracyInNanoseconds);
	ADVRESULT rv = g_Adv2File->AddStatusSection(statusSection);
	AdvProfiling_EndProcessing();
	return rv;
}

ADVRESULT AdvVer2_DefineImageLayout(unsigned char layoutId, const char* layoutType, const char* compression, unsigned char layoutBpp)
{
	if (g_Adv2File == nullptr)
		return E_ADV_NOFILE;

	if (g_Adv2File->ImageSection == nullptr)
		return E_ADV_IMAGE_SECTION_UNDEFINED;

	AdvProfiling_StartProcessing();
	ADVRESULT rv =  g_Adv2File->ImageSection->AddImageLayout(layoutId, layoutType, compression, layoutBpp);		
	AdvProfiling_EndProcessing();
	return rv;
}

ADVRESULT AdvVer2_DefineStatusSectionTag(const char* tagName, int tagType, unsigned int* fileTagId)
{
	if (g_Adv2File == nullptr)
		return E_ADV_NOFILE;

	if (g_Adv2File->StatusSection == nullptr)
		return E_ADV_STATUS_SECTION_UNDEFINED;

	AdvProfiling_StartProcessing();
	ADVRESULT rv = g_Adv2File->StatusSection->DefineTag(tagName, (Adv2TagType)tagType, fileTagId);
	AdvProfiling_EndProcessing();
	return rv;
}

ADVRESULT AdvVer2_AddFileTag(const char* tagName, const char* tagValue)
{
	if (g_Adv2File == nullptr)
		return E_ADV_NOFILE;

	AdvProfiling_StartProcessing();
	ADVRESULT rv = g_Adv2File->AddFileTag(tagName, tagValue);
	AdvProfiling_EndProcessing();
	return rv;
}

ADVRESULT AdvVer2_AddUserTag(const char* tagName, const char* tagValue)
{
	if (g_Adv2File == nullptr)
		return E_ADV_NOFILE;

	AdvProfiling_StartProcessing();
	ADVRESULT rv = g_Adv2File->AddUserTag(tagName, tagValue);
	AdvProfiling_EndProcessing();
	return rv;
}

ADVRESULT AdvVer2_AddOrUpdateImageSectionTag(const char* tagName, const char* tagValue)
{
	if (g_Adv2File == nullptr)
		return E_ADV_NOFILE;

	if (g_Adv2File->ImageSection == nullptr)
		return E_ADV_IMAGE_SECTION_UNDEFINED;
	
	AdvProfiling_StartProcessing();
	ADVRESULT rv = g_Adv2File->ImageSection->AddOrUpdateTag(tagName, tagValue);
	AdvProfiling_EndProcessing();
	return rv;
}

ADVRESULT AdvVer2_FrameAddStatusTagUTF8String(unsigned int tagIndex, const char* tagValue)
{
	if (g_Adv2File == nullptr)
		return E_ADV_NOFILE;

	if (g_Adv2File->StatusSection == nullptr)
		return E_ADV_STATUS_SECTION_UNDEFINED;

	AdvProfiling_StartProcessing();
	ADVRESULT rv = g_Adv2File->StatusSection->AddFrameStatusTagUTF8String(tagIndex, tagValue);
	AdvProfiling_EndProcessing();
	return rv;
}

ADVRESULT AdvVer2_FrameAddStatusTagUInt8(unsigned int tagIndex, unsigned char tagValue)
{
	if (g_Adv2File == nullptr)
		return E_ADV_NOFILE;

	if (g_Adv2File->StatusSection == nullptr)
		return E_ADV_STATUS_SECTION_UNDEFINED;

	AdvProfiling_StartProcessing();
	ADVRESULT rv = g_Adv2File->StatusSection->AddFrameStatusTagUInt8(tagIndex, tagValue);
	AdvProfiling_EndProcessing();
	return rv;
}

ADVRESULT AdvVer2_FrameAddStatusTag16(unsigned int tagIndex, unsigned short tagValue)
{
	if (g_Adv2File == nullptr)
		return E_ADV_NOFILE;

	if (g_Adv2File->StatusSection == nullptr)
		return E_ADV_STATUS_SECTION_UNDEFINED;

	AdvProfiling_StartProcessing();
	ADVRESULT rv = g_Adv2File->StatusSection->AddFrameStatusTagUInt16(tagIndex, tagValue);
	AdvProfiling_EndProcessing();
	return rv;
}

ADVRESULT AdvVer2_FrameAddStatusTagReal(unsigned int tagIndex, float tagValue)
{
	if (g_Adv2File == nullptr)
		return E_ADV_NOFILE;

	if (g_Adv2File->StatusSection == nullptr)
		return E_ADV_STATUS_SECTION_UNDEFINED;
	
	AdvProfiling_StartProcessing();
	ADVRESULT rv = g_Adv2File->StatusSection->AddFrameStatusTagReal(tagIndex, tagValue);
	AdvProfiling_EndProcessing();
	return rv;
}

ADVRESULT AdvVer2_FrameAddStatusTag32(unsigned int tagIndex, unsigned int tagValue)
{
	if (g_Adv2File == nullptr)
		return E_ADV_NOFILE;

	if (g_Adv2File->StatusSection == nullptr)
		return E_ADV_STATUS_SECTION_UNDEFINED;

	AdvProfiling_StartProcessing();
	ADVRESULT rv = g_Adv2File->StatusSection->AddFrameStatusTagUInt32(tagIndex, tagValue);
	AdvProfiling_EndProcessing();
	return rv;
}

ADVRESULT AdvVer2_FrameAddStatusTag64(unsigned int tagIndex, __int64 tagValue)
{
	if (g_Adv2File == nullptr)
		return E_ADV_NOFILE;

	if (g_Adv2File->StatusSection == nullptr)
		return E_ADV_STATUS_SECTION_UNDEFINED;

	AdvProfiling_StartProcessing();
	ADVRESULT rv = g_Adv2File->StatusSection->AddFrameStatusTagUInt64(tagIndex, tagValue);
	AdvProfiling_EndProcessing();
	return rv;
}

/* Assumed pixel format by AdvCore when this method is called

    |    Layout Type    |  ImageLayout.Bpp |  Assumed Pixel Format                                         |
    |  FULL-IMAGE-RAW   |    16, 12, 8     | 16-bit data (1 short per pixel)                               |
    |12BIT-IMAGE-PACKED |    12            | 16-bit data (1 short per pixel) will be packed when storing   |
    
	All other combinations which are not listed above are invalid.
*/
ADVRESULT AdvVer2_FrameAddImage(unsigned char layoutId, unsigned short* pixels, unsigned char pixelsBpp)
{
	if (g_Adv2File == nullptr)
		return E_ADV_NOFILE;

	AdvProfiling_StartProcessing();
	ADVRESULT rv = g_Adv2File->AddFrameImage(layoutId, pixels, pixelsBpp);
	AdvProfiling_EndProcessing();
	return rv;
}

/* Assumed pixel format by AdvCore when this method is called

    |    Layout Type    |  ImageLayout.Bpp |  Assumed Pixel Format                                         |
    |  FULL-IMAGE-RAW   |    16, 12        | 16-bit little endian data passed as bytes (2 bytes per pixel) |
	|  FULL-IMAGE-RAW   |     8            | 8-bit data passed as bytes (1 byte per pixel)                 |
    |12BIT-IMAGE-PACKED |    12            | 12-bit packed data (3 bytes per 2 pixels)                     |
    | 8BIT-COLOR-IMAGE  |     8            | 8-bit RGB or BGR data (3 bytes per pixel, 1 colour per byte)  |

	All other combinations which are not listed above are invalid.
*/
ADVRESULT AdvVer2_FrameAddImageBytes(unsigned char layoutId, unsigned char* pixels, unsigned char pixelsBpp)
{
	if (g_Adv2File == nullptr)
		return E_ADV_NOFILE;

	AdvProfiling_StartProcessing();
	ADVRESULT rv = g_Adv2File->AddFrameImage(layoutId, pixels, pixelsBpp);
	AdvProfiling_EndProcessing();
	return rv;
}

ADVRESULT AdvVer2_GetFramePixels(int streamId, int frameNo, unsigned int* pixels, AdvLib2::AdvFrameInfo* frameInfo, int* systemErrorLen)
{
	if (g_Adv2File == nullptr)
		return E_ADV_NOFILE;

	if (g_Adv2File->ImageSection == nullptr)
		return E_ADV_IMAGE_SECTION_UNDEFINED;

	if (streamId == 0 && frameNo >= g_Adv2File->TotalNumberOfMainFrames)
		return E_FAIL;

	if (streamId > 0 && frameNo >= g_Adv2File->TotalNumberOfCalibrationFrames)
		return E_FAIL;

    unsigned char layoutId;
    enum GetByteMode byteMode;

    ADVRESULT rv = g_Adv2File->GetFrameImageSectionHeader(streamId, frameNo, &layoutId, &byteMode);
	if (rv != S_OK) 
		return rv;

	AdvLib2::Adv2ImageLayout* layout;
	rv = g_Adv2File->ImageSection->GetImageLayoutById(layoutId, &layout);
	if (rv != S_OK) 
		return rv;

    return g_Adv2File->GetFrameSectionData(streamId, frameNo, pixels, frameInfo, systemErrorLen);
}

ADVRESULT AdvVer2_GetStackedFramePixels(int streamId, int frameNo, int framesToStack, bool isSlidingIntegration, unsigned int* pixels, AdvLib2::AdvFrameInfo* frameInfo)
{
	if (g_Adv2File == nullptr)
		return E_ADV_NOFILE;

	if (g_Adv2File->ImageSection == nullptr)
		return E_ADV_IMAGE_SECTION_UNDEFINED;

	int startFrameNo = isSlidingIntegration ? frameNo - framesToStack / 2 : frameNo;
	if (startFrameNo < 0) startFrameNo = 0;
	int endFrameNo = startFrameNo + framesToStack - 1;
	
	if (streamId == 0)
	{
		if (frameNo >= g_Adv2File->TotalNumberOfMainFrames)
			return E_FAIL;
		else
			endFrameNo = min(endFrameNo, g_Adv2File->TotalNumberOfMainFrames - 1);
	}
	else if (streamId > 0)
	{
		if (frameNo >= g_Adv2File->TotalNumberOfCalibrationFrames)
			return E_FAIL;
		else
			endFrameNo = min(endFrameNo, g_Adv2File->TotalNumberOfCalibrationFrames - 1);
	}
	else
	{
		return E_FAIL;
	}

	AdvLib2::AdvFrameInfo frameInfoNext;
	int systemErrorLen;
	int pixelCount = g_Adv2File->ImageSection->Width * g_Adv2File->ImageSection->Height;

	double* stackedPixels = (double*)malloc(pixelCount * sizeof(double));
	ADVRESULT rv = S_OK;

	for (int i = 0; i < pixelCount; i++)
	{
		stackedPixels[i] = 0;
	}

	int stackedFrames = 0;
	unsigned int totalNextExposures = 0;
	for (int i = startFrameNo; i <= endFrameNo; i++)
	{
		unsigned char layoutId;
		enum GetByteMode byteMode;

		rv = g_Adv2File->GetFrameImageSectionHeader(streamId, i, &layoutId, &byteMode);
		if (rv != S_OK) 
			break;

		AdvLib2::Adv2ImageLayout* layout;
		rv = g_Adv2File->ImageSection->GetImageLayoutById(layoutId, &layout);
		if (rv != S_OK) 
			break;

		if (i == frameNo)
			rv = g_Adv2File->GetFrameSectionData(streamId, i, pixels, frameInfo, &systemErrorLen);
		else
		{
			rv = g_Adv2File->GetFrameSectionData(streamId, i, pixels, &frameInfoNext, &systemErrorLen);
			totalNextExposures += frameInfoNext.Exposure;
		}

		if (rv != S_OK) 
			break;

		for (int i = 0; i < pixelCount; i++)
		{
			stackedPixels[i] += pixels[i];
		}
		stackedFrames++;
	}

	if (stackedFrames > 0)
	{
		for (int i = 0; i < pixelCount; i++)
		{
			pixels[i] = (unsigned int)(0.5 + stackedPixels[i] / stackedFrames);
		}

		if (!isSlidingIntegration)
		{
			// Overwrite exposure and timestamps for non-sliding integration;
			frameInfo->EndTicksLo = frameInfoNext.EndTicksLo;
			frameInfo->EndTicksHi = frameInfoNext.EndTicksHi;
		
			__int64 systemTimestampStart = (((__int64)frameInfo->SystemTimestampHi) << 32) + (__int64)frameInfo->SystemTimestampLo;
			__int64 systemTimestampEnd = (((__int64)frameInfoNext.SystemTimestampHi) << 32) + (__int64)frameInfoNext.SystemTimestampLo;
			__int64 systemTimestamp = (systemTimestampStart + systemTimestampEnd) / 2;
			frameInfo->SystemTimestampLo = systemTimestamp & 0xFFFFFFFF;
			frameInfo->SystemTimestampHi = (systemTimestamp >> 32) & 0xFFFFFFFF;

			__int64 utcMidExposureTimestampStart = (((__int64)frameInfo->UtcMidExposureTimestampHi) << 32) + (__int64)frameInfo->UtcMidExposureTimestampLo;
			__int64 utcMidExposureTimestampEnd = (((__int64)frameInfoNext.UtcMidExposureTimestampHi) << 32) + (__int64)frameInfoNext.UtcMidExposureTimestampLo;
			__int64 utcMidExposureTimestamp = (utcMidExposureTimestampStart + utcMidExposureTimestampEnd) / 2;
			frameInfo->UtcMidExposureTimestampLo = utcMidExposureTimestamp & (0xFFFFFFFF);
			frameInfo->UtcMidExposureTimestampHi = (utcMidExposureTimestamp >> 32) & (0xFFFFFFFF);
			frameInfo->Exposure = (int)(utcMidExposureTimestampEnd - (frameInfoNext.Exposure / 2) - (utcMidExposureTimestampStart - (frameInfo->Exposure / 2)));
		}
	}

	delete stackedPixels;
	return rv;
}

ADVRESULT AdvVer2_GetTagPairSizes(TagPairType tagPairType, int tagId, int* tagNameSize, int* tagValueSize)
{
	if (g_Adv2File == nullptr)
		return E_ADV_NOFILE;

	if (tagPairType == TagPairType::MainStream)
		return g_Adv2File->GetMainStreamTagSizes(tagId, tagNameSize, tagValueSize);
	else if (tagPairType == TagPairType::CalibrationStream)
		return g_Adv2File->GetCalibrationStreamTagSizes(tagId, tagNameSize, tagValueSize);
	else if (tagPairType == TagPairType::SystemMetadata)
		return g_Adv2File->GetSystemMetadataTagSizes(tagId, tagNameSize, tagValueSize);
	else if (tagPairType == TagPairType::UserMetadata)
		return g_Adv2File->GetUserMetadataTagSizes(tagId, tagNameSize, tagValueSize);
	else if (tagPairType == TagPairType::ImageSection)
	{
		if (g_Adv2File->ImageSection == nullptr)
			return E_ADV_IMAGE_SECTION_UNDEFINED;
		else
			return g_Adv2File->ImageSection->GetImageSectionTagSizes(tagId, tagNameSize, tagValueSize);
	}
	else if (tagPairType >= TagPairType::FirstImageLayout)
	{
		if (g_Adv2File->ImageSection == nullptr)
			return E_ADV_IMAGE_SECTION_UNDEFINED;
		else
			return g_Adv2File->ImageSection->GetImageLayoutTagSizes(tagPairType - TagPairType::FirstImageLayout, tagId, tagNameSize, tagValueSize);
	}
	return E_FAIL;
}

ADVRESULT AdvVer2_GetTagPairValues(TagPairType tagPairType, int tagId, char* tagName, char* tagValue)
{
	if (g_Adv2File == nullptr)
		return E_ADV_NOFILE;

	if (tagPairType == TagPairType::MainStream)
		return g_Adv2File->GetMainStreamTag(tagId, tagName, tagValue);
	else if (tagPairType == TagPairType::CalibrationStream)
		return g_Adv2File->GetCalibrationStreamTag(tagId, tagName, tagValue);
	else if (tagPairType == TagPairType::SystemMetadata)
		return g_Adv2File->GetSystemMetadataTag(tagId, tagName, tagValue);
	else if (tagPairType == TagPairType::UserMetadata)
		return g_Adv2File->GetUserMetadataTag(tagId, tagName, tagValue);
	else if (tagPairType == TagPairType::ImageSection)
	{
		if (g_Adv2File->ImageSection == nullptr)
			return E_ADV_IMAGE_SECTION_UNDEFINED;
		else
			return g_Adv2File->ImageSection->GetImageSectionTag(tagId, tagName, tagValue);
	}
	else if (tagPairType >= TagPairType::FirstImageLayout)
	{
		if (g_Adv2File->ImageSection == nullptr)
			return E_ADV_IMAGE_SECTION_UNDEFINED;
		else
			return g_Adv2File->ImageSection->GetImageLayoutTag(tagPairType - TagPairType::FirstImageLayout, tagId, tagName, tagValue);
	}
	return E_FAIL;
}

ADVRESULT AdvVer2_GetStatusTagNameSize(unsigned int tagId, int* tagNameSize)
{
	if (g_Adv2File == nullptr)
	return E_ADV_NOFILE;

	if (g_Adv2File->StatusSection == nullptr)
		return E_ADV_STATUS_SECTION_UNDEFINED;

	return g_Adv2File->StatusSection->GetStatusTagNameSize(tagId, tagNameSize);
}

ADVRESULT AdvVer2_GetStatusTagInfo(unsigned int tagId, char* tagName, Adv2TagType* tagType)
{
	if (g_Adv2File == nullptr)
		return E_ADV_NOFILE;
	
	if (g_Adv2File->StatusSection == nullptr)
		return E_ADV_STATUS_SECTION_UNDEFINED;

	return g_Adv2File->StatusSection->GetStatusTagInfo(tagId, tagName, tagType);
}

ADVRESULT AdvVer2_GetStatusTagSizeUTF8String(unsigned int tagIndex, int* tagValueSize)
{
	if (g_Adv2File == nullptr)
		return E_ADV_NOFILE;

	if (g_Adv2File->StatusSection == nullptr)
		return E_ADV_STATUS_SECTION_UNDEFINED;

	return g_Adv2File->StatusSection->GetStatusTagSizeUTF8String(tagIndex, tagValueSize);
}

ADVRESULT AdvVer2_GetStatusTagUTF8String(unsigned int tagIndex, char* tagValue)
{
	if (g_Adv2File == nullptr)
		return E_ADV_NOFILE;

	if (g_Adv2File->StatusSection == nullptr)
		return E_ADV_STATUS_SECTION_UNDEFINED;

	return g_Adv2File->StatusSection->GetStatusTagUTF8String(tagIndex, tagValue);
}

ADVRESULT AdvVer2_GetStatusTagUInt8(unsigned int tagIndex, unsigned char* tagValue)
{
	if (g_Adv2File == nullptr)
		return E_ADV_NOFILE;

	if (g_Adv2File->StatusSection == nullptr)
		return E_ADV_STATUS_SECTION_UNDEFINED;

	return g_Adv2File->StatusSection->GetStatusTagUInt8(tagIndex, tagValue);
}

ADVRESULT AdvVer2_GetStatusTag16(unsigned int tagIndex, unsigned short* tagValue)
{
	if (g_Adv2File == nullptr)
		return E_ADV_NOFILE;
	
	if (g_Adv2File->StatusSection == nullptr)
		return E_ADV_STATUS_SECTION_UNDEFINED;

	return g_Adv2File->StatusSection->GetStatusTag16(tagIndex, tagValue);
}

ADVRESULT AdvVer2_GetStatusTagReal(unsigned int tagIndex, float* tagValue)
{
	if (g_Adv2File == nullptr)
		return E_ADV_NOFILE;

	if (g_Adv2File->StatusSection == nullptr)
		return E_ADV_STATUS_SECTION_UNDEFINED;

	return g_Adv2File->StatusSection->GetStatusTagReal(tagIndex, tagValue);
}

ADVRESULT AdvVer2_GetStatusTag32(unsigned int tagIndex, unsigned int* tagValue)
{
	if (g_Adv2File == nullptr)
		return E_ADV_NOFILE;

	if (g_Adv2File->StatusSection == nullptr)
		return E_ADV_STATUS_SECTION_UNDEFINED;

	return g_Adv2File->StatusSection->GetStatusTag32(tagIndex, tagValue);
}

ADVRESULT AdvVer2_GetStatusTag64(unsigned int tagIndex, __int64* tagValue)
{
	if (g_Adv2File == nullptr)
		return E_ADV_NOFILE;

	if (g_Adv2File->StatusSection == nullptr)
		return E_ADV_STATUS_SECTION_UNDEFINED;

	return g_Adv2File->StatusSection->GetStatusTag64(tagIndex, tagValue);
}

ADVRESULT AdvVer2_GetImageLayoutInfo(int layoutIndex, AdvLib2::AdvImageLayoutInfo* imageLayoutInfo)
{
	if (g_Adv2File == nullptr)
		return E_ADV_NOFILE;

	if (g_Adv2File->ImageSection == nullptr)
		return E_ADV_IMAGE_SECTION_UNDEFINED;

	return g_Adv2File->ImageSection->GetImageLayoutInfo(layoutIndex, imageLayoutInfo);
}

ADVRESULT AdvVer2_GetIndexEntries(AdvLib2::AdvIndexEntry* mainIndex, AdvLib2::AdvIndexEntry* calibrationIndex)
{
	if (g_Adv2File == nullptr)
		return E_ADV_NOFILE;

	return g_Adv2File->GetIndexEntries(mainIndex, calibrationIndex);
}

int AdvVer2_GetLastSystemSpecificFileError()
{
	if (g_Adv2File == nullptr)
		return 0;
	else
		return g_Adv2File->GetLastSystemSpecificFileError();
}