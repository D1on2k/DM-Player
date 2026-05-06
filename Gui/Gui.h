#pragma once

#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <windows.h>
#include "ImGui/imgui.h"
#include <d3d11.h>
#include "stb_image.h"
#include <string>

ID3D11Device* g_pd3dDevice = nullptr;
ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
IDXGISwapChain* g_pSwapChain = nullptr;
bool g_SwapChainOccluded = false;
UINT g_ResizeWidth = 0, g_ResizeHeight = 0;
ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
int selectedIndex = -1;

bool test = true;

static int Tabsystem = 0;
static int LibraryTabSystem = 0;

//std::string HoldSearch = "";
char HoldSearch[128] = ""; // Make it char because compiler hates string

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
