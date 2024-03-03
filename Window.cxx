/*
Copyright (c) 2024 Eugene Kirian

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Defines
#define WINDOW_CLASS_NAME L"GDIExample"
#define WINDOW_TITLE_NAME L"GDI Example"

// Error Codes
#define ERROR_NONE 0
#define ERROR_CREATE_WINDOW_CLASS 1
#define ERROR_CREATE_WINDOW_INSTANCE 2

INT Width = 800;
INT Height = 600;

BOOL Resize = FALSE;

HBITMAP CreateBitmapAndFillPtrToItsData(BYTE** data)
{
    HDC hdcScreen = GetDC(NULL);

    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(BITMAPINFO));

    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = Width;
    bmi.bmiHeader.biHeight = -Height; // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    HBITMAP bmp = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, (VOID**)data, NULL, NULL);

    ReleaseDC(NULL, hdcScreen);

    return bmp;
}

void CopyInPixelData(BYTE* pixels)
{
    CONST INT c_x = Width / 2;
    CONST INT c_y = Height / 2;

    CONST INT radius = min(c_x, c_y);
    CONST INT radius2 = radius * radius;

    INT i = 0;

    for (INT y = 0; y < Height; y++)
    {
        for (INT x = 0; x < Width; x++)
        {
            if ((x - c_x) * (x - c_x) + (y - c_y) * (y - c_y) <= radius2)
            {
                pixels[i++] = 64;
                pixels[i++] = 64;
                pixels[i++] = 64;
                pixels[i++] = 0;
            }
            else
            {
                pixels[i++] = 0;
                pixels[i++] = 0;
                pixels[i++] = 0;
                pixels[i++] = 0;
            }
        }
    }
}

HBITMAP CreateBitmapFromPixelDataExample()
{
    BYTE* pixels = NULL;

    // Create a bitmap such that we get a pointer to where its data is stored
    HBITMAP bitmap = CreateBitmapAndFillPtrToItsData(&pixels);

    // Fill in some pixel data...
    CopyInPixelData(pixels);

    return bitmap;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HBITMAP bitmap;

    switch (message)
    {
    case WM_ERASEBKGND: { return 1; }
    case WM_CREATE: { bitmap = CreateBitmapFromPixelDataExample(); break; }
    case WM_CLOSE: { PostQuitMessage(0); break; }
    case WM_PAINT:
    {
        if (Resize) { bitmap = CreateBitmapFromPixelDataExample(); }

        RECT rect;
        GetClientRect(hWnd, &rect);

        HDC hdc_bitmap = CreateCompatibleDC(NULL); // TODO do not allocate bitmap every time...
        HBITMAP hbm_old = (HBITMAP)SelectObject(hdc_bitmap, bitmap);

        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        // Paint in the bitmap we generated from pixel data...
        BitBlt(hdc, 0, 0, Width, Height, hdc_bitmap, 0, 0, SRCCOPY);
        EndPaint(hWnd, &ps);

        SelectObject(hdc_bitmap, hbm_old);
        DeleteDC(hdc_bitmap);

        break;
    }
    case WM_SIZE: { Width = LOWORD(lParam); Height = HIWORD(lParam); Resize = TRUE; break; }
    default: { return DefWindowProc(hWnd, message, wParam, lParam); }
    }

    return 0;
}

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow)
{
    DWORD processorId = 0;
    SetThreadIdealProcessor(GetCurrentThread(), processorId);

    MSG msg = { 0 };
    WNDCLASS wc = { 0 };

    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
    wc.lpszClassName = WINDOW_CLASS_NAME;

    if (!RegisterClass(&wc)) { return ERROR_CREATE_WINDOW_CLASS; }

    HWND hwnd = CreateWindow(wc.lpszClassName,
        WINDOW_TITLE_NAME,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        200, 300, Width, Height, 0, 0, hInstance, NULL);

    if (hwnd == NULL) { return ERROR_CREATE_WINDOW_INSTANCE; }

    while (WM_QUIT != msg.message)
    {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // TODO: Handle minimized state.
        {
            RECT rect;
            GetClientRect(hwnd, &rect);

            InvalidateRect(hwnd, &rect, TRUE);
            UpdateWindow(hwnd);
        }
    }

    return ERROR_NONE;
}