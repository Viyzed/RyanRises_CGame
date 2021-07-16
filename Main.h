#pragma once

#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#define GAME_NAME       "RYAN_RISES"
#define GAME_RES_WIDTH  384
#define GAME_RES_HEIGHT 240

#define GAME_BPP        32
#define GAME_DRAWING_AREA_MEMORY_SIZE   (GAME_RES_WIDTH * GAME_RES_HEIGHT * (GAME_BPP / 8))

#define CALCULATE_AVERAGE_FPS_AFTER_X_FRAMES    100
#define TARGET_MICROSECONDS_PER_FRAME   16667

//#define SIMD

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

    LONG MinimumTimerResolution;
    LONG MaximumTimerResolution;
    LONG CurrentTimerResolution;

} PERF_DATA;

LRESULT CALLBACK MainWndowProc(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam);

DWORD CreateMainGameWindow(void);

BOOL GameRunning(void);

void ProcessPlayerInput(void);

void RenderFrameGraphics(void);

#ifdef SIMD
void ClearScreen(__m128i Colour);
#else
void ClearScreen(PIXEL32* Colour);
#endif // SIMD


#endif // MAIN_H_INCLUDED
