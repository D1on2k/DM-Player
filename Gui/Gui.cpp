/*
MIT License

Copyright (c) 2026 Dion2k

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

#pragma once

#include <windows.h>
#include <d3d11.h>
#include <tlhelp32.h>
#include <string>
#include <iostream>
#include <vector>
#include "ImGui/imgui.h"
#include "ImGui/imconfig.h"
#include "ImGui/backends/imgui_impl_win32.h"
#include "ImGui/backends/imgui_impl_dx11.h"
#include "ImGui/ImGui.h"
#include "ImGui/imgui_internal.h"
#include "Searching/searching.h"
#include "PathTab4/select_path.h"
#include "AudioScanner/audioscan.h"
#include "AudioPlay/play.h"

/*
ID3D11Device* g_pd3dDevice = nullptr;
ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
IDXGISwapChain* g_pSwapChain = nullptr;
UINT g_ResizeWidth = 0, g_ResizeHeight = 0;
ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
*/

extern ID3D11Device* g_pd3dDevice;
extern ID3D11DeviceContext* g_pd3dDeviceContext;
extern IDXGISwapChain* g_pSwapChain;
extern UINT g_ResizeWidth, g_ResizeHeight;
extern ID3D11RenderTargetView* g_mainRenderTargetView;

/* Declering them here kept getting me compiler errors
static bool songsLoaded = false;
static bool idk = true;
static bool scan = false;
bool g_SwapChainOccluded = false;
bool test = true;
bool g_IsMaximized = false;

RECT g_WindowRectWhenNormal = {0};
RECT g_NormalRect = { 0, 0, 0, 0 };

static int Tabsystem = 0;
static int LibraryTabSystem = 0;
int selectedIndex = -1;
*/

extern bool songsLoaded;
extern bool idk;
extern bool scan;
extern bool g_SwapChainOccluded;
extern bool test;
extern bool g_IsMaximized;

extern RECT g_WindowRectWhenNormal;
extern RECT g_NormalRect;

extern int Tabsystem;
extern int LibraryTabSystem;
extern int selectedIndex;


//std::string HoldSearch = "";
extern char pathBuffer[512]; // = {}
extern char HoldSearch[128]; // Make it char because compiler hates string
extern char IAMTIRED[256];

struct SongDisplay
{
    std::string title;
    std::string artist;
    std::string album;
    std::string path;
};

extern std::vector<SongDisplay> songlist;
extern std::vector<std::string> songtitles;

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
void ToggleMaximize(HWND hwnd);

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

