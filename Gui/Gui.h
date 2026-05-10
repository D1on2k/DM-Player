#pragma once

#define STB_IMAGE_IMPLEMENTATION

#include <windows.h>
#include <d3d11.h>
#include <tlhelp32.h>
#include <string>
#include <iostream>
#include "ImGui/imgui.h"
#include "ImGui/imconfig.h"
#include "ImGui/backends/imgui_impl_win32.h"
#include "ImGui/backends/imgui_impl_dx11.h"
#include "ImGui/ImGui.h"
#include "ImGui/imgui_internal.h"
#include "stb_image.h"
#include "Searching/searching.h"
#include "PathTab4/select_path.h"


ID3D11Device* g_pd3dDevice = nullptr;
ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
IDXGISwapChain* g_pSwapChain = nullptr;
UINT g_ResizeWidth = 0, g_ResizeHeight = 0;
ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

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

//std::string HoldSearch = "";
char HoldSearch[128] = ""; // Make it char because compiler hates string
char pathBuffer[512] = {};
extern char IAMTIRED[256];

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
void ToggleMaximize(HWND hwnd);

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
