/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADV_CROSS_PLATFORM
#define ADV_CROSS_PLATFORM

#define _FILE_OFFSET_BITS  64
#include "stdio.h"
#include "adv2_error_codes.h"

#ifndef _WIN32
typedef int long long __int64;
typedef unsigned int long long __uint64;

typedef int BOOL;
typedef int LONG;
typedef unsigned short int WORD;
typedef unsigned char BYTE;
typedef unsigned int DWORD;

#define BI_RGB 0

typedef void* HBITMAP;

#pragma pack(1)
typedef struct tagBITMAPINFOHEADER {
  DWORD biSize;
  int  biWidth;
  int  biHeight;
  WORD  biPlanes;
  WORD  biBitCount;
  DWORD biCompression;
  DWORD biSizeImage;
  int  biXPelsPerMeter;
  int  biYPelsPerMeter;
  DWORD biClrUsed;
  DWORD biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;
#pragma pack()

#pragma pack(1)
typedef struct tagBITMAPFILEHEADER {
  WORD  bfType;
  DWORD bfSize;
  WORD  bfReserved1;
  WORD  bfReserved2;
  DWORD bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;
#pragma pack()

#endif

#if __linux__
#include <time.h>
#endif

#if __APPLE__
#ifndef _SIZE_T_DEFINED
#define _SIZE_T_DEFINED
#endif
#endif

#if __GNUC__
#define strcpy_s(x, y, z) strcpy(x, z)
#define strncpy_s(x, y, z, t) strcpy(x, z)
#define _snprintf_s(x, y, z, t) snprintf(x, y, z, t);
void fopen_s(FILE **f, const char *name, const char *mode);
#endif

#if _MSC_VER
#define snprintf _snprintf
#define MSVC
// Including SDKDDKVer.h defines the highest available Windows platform.

// If you wish to build your application for a previous Windows platform, include WinSDKVer.h and
// set the _WIN32_WINNT macro to the platform you wish to support before including SDKDDKVer.h.

#include <SDKDDKVer.h>

#endif

#if defined _WIN32 || defined _WIN64 || defined __CYGWIN__
 /*
  #ifdef BUILDING_DLL
    #ifdef __GNUC__
      #define DLL_PUBLIC __attribute__ ((dllexport))
    #else
      #define DLL_PUBLIC __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
    #endif
  #else
    #ifdef __GNUC__
      #define DLL_PUBLIC __attribute__ ((dllimport))
    #else
      #define DLL_PUBLIC __declspec(dllimport) // Note: actually gcc seems to also supports this syntax.
    #endif
  #endif
   */
    #define DLL_PUBLIC __declspec(dllexport)
    #define DLL_LOCAL   
#else
  #if __GNUC__ >= 4
    #define DLL_PUBLIC __attribute__ ((visibility ("default")))
    #define DLL_LOCAL  __attribute__ ((visibility ("hidden")))
  #else
    #define DLL_PUBLIC __declspec(dllexport)
    #define DLL_LOCAL
  #endif
#endif

FILE* advfopen(const char* fileName, const char* modes);
size_t advfwrite(const void* pData, size_t size, size_t count, FILE* file);
void advfread(void* pData, size_t size, size_t count, FILE* file);
void advfgetpos64(FILE* file, __int64* pos);
int advfsetpos64(FILE* file, const __int64* pos);
int advfsetpos64(FILE* file, const __int64* pos, int origin);
int advfseek(FILE* stream, __int64 off, int whence);
int advfclose(FILE* file);
int advfflush(FILE* file);

__int64 advgetclockresolution();
__int64 advgetclockticks();

ADVRESULT CheckFileName(const char* fileName, bool overwriteExisting);

#endif