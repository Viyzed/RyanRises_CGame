#pragma once

#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#define GAME_NAME       "RYAN_RISES"
#define GAME_RES_WIDTH  384
#define GAME_RES_HEIGHT 240

#define GAME_BPP        32
#define GAME_DRAWING_AREA_MEMORY_SIZE   (GAME_RES_WIDTH * GAME_RES_HEIGHT * (GAME_BPP / 8))

#define CALCULATE_AVERAGE_FPS_AFTER_X_FRAMES    120
#define TARGET_MICROSECONDS_PER_FRAME   16667ULL

#define SIMD

typedef LONG(NTAPI* _NTQueryTimerResolution) (OUT PULONG MinimumResolution, OUT PULONG MaximumResolution, OUT PULONG CurrentResolution);
_NTQueryTimerResolution NTQueryTimerResolution;

typedef struct GAMEBITMAP
{

    BITMAPINFO BitmapInfo; //44 Bytes
    void* Memory; //8 Bytes

} GAMEBITMAP;

typedef struct PIXEL32
{
    uint8_t Blue;
    uint8_t Green;
    uint8_t Red;
    uint8_t Alpha;

} PIXEL32;

typedef struct PERF_DATA
{

    uint64_t TotalFramesRendered;

    float RawFPSAverage;
    float CookedFPSAverage;

    int64_t Frequency;

    MONITORINFO MonitorInfo;

    int32_t MonitorWidth;
    int32_t MonitorHeight;

    BOOL DiaplayDebugInfo;

    ULONG MinimumTimerResolution;
    ULONG MaximumTimerResolution;
    ULONG CurrentTimerResolution;

    DWORD HandleCount; 

    DWORD PrivateBytes;
    PROCESS_MEMORY_COUNTERS_EX MemInfo;

    SYSTEM_INFO SystemInfo;
    int64_t PreviousSystemTime;
    int64_t CurrentSystemTime;
    double CPUPercent;

} PERF_DATA;

typedef struct PLAYER
{
    char Name[12];
    int32_t ScreenPosX;
    int32_t ScreenPosY;
    int32_t HP;
    int32_t Strength;
    int32_t MP;

} PLAYER;

LRESULT CALLBACK MainWndowProc(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam);

DWORD CreateMainGameWindow(void);

BOOL GameRunning(void);

void ProcessPlayerInput(void);

void RenderFrameGraphics(void);

DWORD Load32BppBitmapFromFile(_In_ char* FileName, _Inout_ GAMEBITMAP* GameBitmap);

#ifdef SIMD
void ClearScreen(__m128i* Colour);
#else
void ClearScreen(PIXEL32* Colour);
#endif // SIMD


#endif // MAIN_H_INCLUDED
