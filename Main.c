#include <stdio.h>
#include <stdint.h>

#include <Windows.h>

#include <emmintrin.h>

#include "Main.h"

BOOL gGameIsRunning;

HWND gGameWindow;

GAMEBITMAP gBackBuffer;

PERF_DATA gPerformanceData;

int APIENTRY WinMain(HINSTANCE Instance, HINSTANCE PreviousInstance, PSTR CommandLine, int CommandShow)
{
    UNREFERENCED_PARAMETER(Instance);
    UNREFERENCED_PARAMETER(PreviousInstance);
    UNREFERENCED_PARAMETER(CommandLine);
    UNREFERENCED_PARAMETER(CommandShow);

    MSG Message = { 0 };

    int64_t FrameStart = 0;
    int64_t FrameEnd = 0;
    int64_t ElapsedMicrosecondsPerFrameAccumulatorRaw = 0;
    int64_t ElapsedMicrosecondsPerFrameAccumulatorCooked = 0;
    int64_t ElapsedMicrosecondsPerFrame = 0;

    //Undocummented function NtQueryTimerResolution() imported from  C:\Windows\System32\ntdll.dll
    HMODULE NtDllModuleHandle;

    if ((NtDllModuleHandle = GetModuleHandleA("ntdll.dll")) == NULL)
    {
        MessageBoxA(NULL, "Couldn't load ntdll.dll", "Error.", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    if ((NTQueryTimerResolution = (_NTQueryTimerResolution)GetProcAddress(NtDllModuleHandle, "NtQueryTimerResolution")) == NULL)
    {
        MessageBoxA(NULL, "Couldn't find the NtQueryTimerResolution function in ntdll.dll", "Error.", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    NTQueryTimerResolution(&gPerformanceData.MinimumTimerResolution, &gPerformanceData.MaximumTimerResolution, &gPerformanceData.CurrentTimerResolution);

    if (GameRunning() == TRUE)
    {
        MessageBoxA(NULL, "Game already running...", "Error", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    if (CreateMainGameWindow() != ERROR_SUCCESS)
    {
        goto Exit;
    }

    QueryPerformanceFrequency((LARGE_INTEGER*)&gPerformanceData.Frequency);

    gBackBuffer.BitmapInfo.bmiHeader.biSize = sizeof(gBackBuffer.BitmapInfo.bmiHeader);
    gBackBuffer.BitmapInfo.bmiHeader.biWidth = GAME_RES_WIDTH;
    gBackBuffer.BitmapInfo.bmiHeader.biHeight = GAME_RES_HEIGHT;
    gBackBuffer.BitmapInfo.bmiHeader.biBitCount = GAME_BPP;
    gBackBuffer.BitmapInfo.bmiHeader.biCompression = BI_RGB;
    gBackBuffer.BitmapInfo.bmiHeader.biPlanes = 1;

    gBackBuffer.Memory = VirtualAlloc(NULL, GAME_DRAWING_AREA_MEMORY_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if (gBackBuffer.Memory == NULL)
    {
        MessageBox(NULL, "Failed to allocate memory for drawing surface.", "Error", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    //Main Game Loop

    gGameIsRunning = TRUE;

    while (gGameIsRunning == TRUE)
    {
        QueryPerformanceCounter((LARGE_INTEGER*)&FrameStart);

        while (PeekMessageA(&Message, gGameWindow, 0, 0, PM_REMOVE))
        {
            DispatchMessageA(&Message);
        }

        ProcessPlayerInput();

        RenderFrameGraphics();

        QueryPerformanceCounter((LARGE_INTEGER*)&FrameEnd);

        ElapsedMicrosecondsPerFrame = FrameEnd - FrameStart;

        ElapsedMicrosecondsPerFrame *= 1000000;

        ElapsedMicrosecondsPerFrame /= gPerformanceData.Frequency;

        gPerformanceData.TotalFramesRendered++;

        ElapsedMicrosecondsPerFrameAccumulatorRaw += ElapsedMicrosecondsPerFrame;

        while (ElapsedMicrosecondsPerFrame <= TARGET_MICROSECONDS_PER_FRAME)
        {

            ElapsedMicrosecondsPerFrame = FrameEnd - FrameStart;

            ElapsedMicrosecondsPerFrame *= 1000000;

            ElapsedMicrosecondsPerFrame /= gPerformanceData.Frequency;

            QueryPerformanceCounter((LARGE_INTEGER*)&FrameEnd);

            if (ElapsedMicrosecondsPerFrame <= ((int64_t)TARGET_MICROSECONDS_PER_FRAME - gPerformanceData.CurrentTimerResolution))
            {
                Sleep(1);
            }

        }

        ElapsedMicrosecondsPerFrameAccumulatorCooked += ElapsedMicrosecondsPerFrame;

        if (gPerformanceData.TotalFramesRendered % CALCULATE_AVERAGE_FPS_AFTER_X_FRAMES == 0)
        {
            gPerformanceData.RawFPSAverage = 1.0f / ((ElapsedMicrosecondsPerFrameAccumulatorRaw / CALCULATE_AVERAGE_FPS_AFTER_X_FRAMES) * 0.000001f);
            gPerformanceData.CookedFPSAverage = 1.0f / ((ElapsedMicrosecondsPerFrameAccumulatorCooked / CALCULATE_AVERAGE_FPS_AFTER_X_FRAMES) * 0.000001f);

            ElapsedMicrosecondsPerFrameAccumulatorRaw = 0;
            ElapsedMicrosecondsPerFrameAccumulatorCooked = 0;
        }

    }

Exit:

    return(0);

}

LRESULT CALLBACK MainWndowProc(
    HWND WindowHandle,      // handle to window
    UINT Message,           // message identifier
    WPARAM WParam,          // first message parameter
    LPARAM LParam)          // second message parameter
{

    LRESULT Result = 0;

    switch (Message)
    {
        // Quit the program when 'X' is clicked.
    case WM_CLOSE:
    {
        gGameIsRunning = FALSE;
        PostQuitMessage(0);
        break;
    }
    //
    // Process other messages.
    //

    default:
        Result = DefWindowProcA(WindowHandle, Message, WParam, LParam);
    }
    return(Result);
}

DWORD CreateMainGameWindow()
{
    DWORD Result = ERROR_SUCCESS;

    WNDCLASSEXA WindowClass = { 0 };

    WindowClass.cbSize = sizeof(WNDCLASSEXA);
    WindowClass.style = 0;
    WindowClass.lpfnWndProc = MainWndowProc;
    WindowClass.cbClsExtra = 0;
    WindowClass.cbWndExtra = 0;
    WindowClass.hInstance = GetModuleHandleA(NULL);
    WindowClass.hIcon = LoadIconA(NULL, IDI_APPLICATION);
    WindowClass.hCursor = LoadCursorA(NULL, IDC_ARROW);
    WindowClass.hbrBackground = CreateSolidBrush(RGB(255, 0, 255));
    WindowClass.lpszMenuName = NULL;
    WindowClass.lpszClassName = GAME_NAME "_GAME_WINDOWCLASS";
    WindowClass.hIconSm = LoadIconA(NULL, IDI_APPLICATION);

    //SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    if (RegisterClassExA(&WindowClass) == 0)
    {
        Result = GetLastError();
        MessageBox(NULL, "Window Registration Failed", "Error", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    gGameWindow = CreateWindowExA(0, WindowClass.lpszClassName, GAME_NAME, WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, NULL, NULL, GetModuleHandleA(NULL), NULL);

    if (gGameWindow == NULL)
    {
        Result = GetLastError();
        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    gPerformanceData.MonitorInfo.cbSize = sizeof(MONITORINFO);
    if (GetMonitorInfoA(MonitorFromWindow(gGameWindow, MONITOR_DEFAULTTOPRIMARY), &gPerformanceData.MonitorInfo) == 0)
    {
        Result = MONITOR_DEFAULTTONULL;

        goto Exit;
    }

    gPerformanceData.MonitorWidth = gPerformanceData.MonitorInfo.rcMonitor.right - gPerformanceData.MonitorInfo.rcMonitor.left;
    gPerformanceData.MonitorHeight = gPerformanceData.MonitorInfo.rcMonitor.bottom - gPerformanceData.MonitorInfo.rcMonitor.top;

    if (SetWindowLongPtrA(gGameWindow, GWL_STYLE, WS_VISIBLE) == 0)
    {
        Result = GetLastError();
        goto Exit;
    }
    if (SetWindowPos(gGameWindow, HWND_TOP, gPerformanceData.MonitorInfo.rcMonitor.left,
        gPerformanceData.MonitorInfo.rcMonitor.top, gPerformanceData.MonitorWidth,
        gPerformanceData.MonitorHeight, SWP_FRAMECHANGED) == 0)
    {
        Result = GetLastError();
        goto Exit;
    }

Exit:

    return(Result);
}


BOOL GameRunning() {
#ifdef CHECK
    HANDLE Mutex = NULL;
    Mutex = CreateMutexA(NULL, FALSE, GAME_NAME "_GameMutex");
#endif // CHECK

    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        return(TRUE);
    }
    else
    {
        return(FALSE);
    }
}

void ProcessPlayerInput()
{
    int16_t EscapeKeyIsDown = GetAsyncKeyState(VK_ESCAPE);

    int16_t DebugKeyIsDown = GetAsyncKeyState(VK_F3);
    static int16_t DebugKeyWasDown;

    if (EscapeKeyIsDown != 0)
    {
        SendMessageA(gGameWindow, WM_CLOSE, 0, 0);
    }

    if (DebugKeyIsDown != 0 && DebugKeyWasDown == 0) {
        gPerformanceData.DiaplayDebugInfo = !gPerformanceData.DiaplayDebugInfo;
    }

    DebugKeyWasDown = DebugKeyIsDown;
}

void RenderFrameGraphics()
{
    __m128i QuadPixel = { 0x7f, 0x00, 0x0A, 0xff, 0x7f, 0x00, 0x0A, 0xff, 0x7f, 0x00, 0x0A, 0xff, 0x7f, 0x00, 0x0A, 0xff };

#ifdef SIMD
    ClearScreen(QuadPixel);
#else
    PIXEL32 Pixel = { 0x7f, 0x00, 0x00, 0xff };
    ClearScreen(&Pixel);
#endif //SIMD

    int32_t ScreenX = 25;
    int32_t ScreenY = 25;

    //X-Y Coordinate system.
    int32_t StartingScreenPixel = ((GAME_RES_WIDTH * GAME_RES_HEIGHT) - GAME_RES_WIDTH) - (GAME_RES_WIDTH * ScreenY) + ScreenX;

    //Render tile/sprite.
    for (int32_t y = 0; y < 16; y++)
    {
        for (int32_t x = 0; x < 16; x++)
        {
            memset((PIXEL32*)gBackBuffer.Memory + (uintptr_t)StartingScreenPixel + x - ((uintptr_t)GAME_RES_WIDTH * y), 0xFF, sizeof(PIXEL32));
        }
    }

    HDC DeviceContext = GetDC(gGameWindow);

    StretchDIBits(DeviceContext,
        0,
        0,
        gPerformanceData.MonitorWidth,
        gPerformanceData.MonitorHeight,
        0,
        0,
        GAME_RES_WIDTH,
        GAME_RES_HEIGHT,
        gBackBuffer.Memory,
        &gBackBuffer.BitmapInfo,
        DIB_RGB_COLORS,
        SRCCOPY);

    if (gPerformanceData.DiaplayDebugInfo == TRUE)
    {
        //Monospaced font for debug Text.
        SelectObject(DeviceContext, (HFONT)GetStockObject(ANSI_FIXED_FONT));

        //Text buffer for writing debug information into.
        char DebugTextBuffer[64] = { 0 };

        //Write debug text with Frames Per Second RAW and Processed to screen (F3)
        sprintf_s(DebugTextBuffer, _countof(DebugTextBuffer), "FPS Raw:    %.01f", gPerformanceData.RawFPSAverage);
        TextOutA(DeviceContext, 0, 0, DebugTextBuffer, strlen(DebugTextBuffer));

        sprintf_s(DebugTextBuffer, _countof(DebugTextBuffer), "FPS Cooked: %.01f", gPerformanceData.CookedFPSAverage);
        TextOutA(DeviceContext, 0, 13, DebugTextBuffer, strlen(DebugTextBuffer));

        //Write NtDLL Clock Timer Resolutions to screen.
        sprintf_s(DebugTextBuffer, _countof(DebugTextBuffer), "Min. Timer Res. %.02f", gPerformanceData.MinimumTimerResolution / 10000.0f);
        TextOutA(DeviceContext, 0, 26, DebugTextBuffer, strlen(DebugTextBuffer));

        sprintf_s(DebugTextBuffer, _countof(DebugTextBuffer), "Max. Timer Res. %.02f", gPerformanceData.MaximumTimerResolution / 10000.0f);
        TextOutA(DeviceContext, 0, 39, DebugTextBuffer, strlen(DebugTextBuffer));

        sprintf_s(DebugTextBuffer, _countof(DebugTextBuffer), "Cur. Timer Res. %.02f", gPerformanceData.CurrentTimerResolution / 10000.0f);
        TextOutA(DeviceContext, 0, 52, DebugTextBuffer, strlen(DebugTextBuffer));
    }

    ReleaseDC(gGameWindow, DeviceContext);
}

//Render background onto the Canvas (Back Buffer).
#ifdef SIMD
__forceinline void ClearScreen(__m128i Colour)
{
    for (int i = 0; i < GAME_RES_WIDTH * GAME_RES_HEIGHT; i += 4)
    {
        _mm_store_si128((PIXEL32*)gBackBuffer.Memory + i, Colour);
    }
}
#else
__forceinline void ClearScreen(PIXEL32* Pixel)
{
    for (int i = 0; i < GAME_RES_WIDTH * GAME_RES_HEIGHT; i++)
    {
        memcpy((PIXEL32*)gBackBuffer.Memory + i, Pixel, sizeof(PIXEL32));
    }
}
#endif // SIMD
