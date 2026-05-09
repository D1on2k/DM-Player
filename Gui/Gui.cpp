// Gui.cpp

#include <windows.h>
#include "ImGui/imgui.h"
#include "ImGui/imconfig.h"
#include "ImGui/backends/imgui_impl_win32.h"
#include "ImGui/backends/imgui_impl_dx11.h"
#include "ImGui/ImGui.h"
#include "ImGui/imgui_internal.h"
#include "Gui.h"
#include "Searching/searching.h"
#include <tlhelp32.h>

using namespace ImGui;

extern ID3D11Device* g_pd3dDevice;
extern ID3D11DeviceContext* g_pd3dDeviceContext;
extern IDXGISwapChain* g_pSwapChain;
extern bool g_SwapChainOccluded;
extern UINT g_ResizeWidth, g_ResizeHeight;
extern ID3D11RenderTargetView* g_mainRenderTargetView;

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

ID3D11ShaderResourceView* LoadTexture(const char* filename, ID3D11Device* device) 
{
    int width, height, channels;
    
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 4); // Open a file and read its RGB values height everything about the image
    
    if (data == NULL) return nullptr;

    // Create the Texture2D
    D3D11_TEXTURE2D_DESC desc = {}; // Tell the GUI that I will send the following : width height image all 
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    ID3D11Texture2D* pTexture = nullptr; // Move the image from my ram to my vram 
    D3D11_SUBRESOURCE_DATA subResource = { data, (UINT)(width * 4), 0 };
    device->CreateTexture2D(&desc, &subResource, &pTexture);

    // Create a Resource View id that i will use later in ImGui
    ID3D11ShaderResourceView* srv = nullptr;
    device->CreateShaderResourceView(pTexture, nullptr, &srv);
    
    // Just a bit of cleaning up the ram after its been sent to the gpu
    pTexture->Release();
    stbi_image_free(data);
    return srv;
}

void MakeTheWindowRound(HWND hwnd, int radius)
{
    RECT rect;
    GetClientRect(hwnd, &rect);
    HRGN rgn = CreateRoundRectRgn(0, 0, rect.right, rect.bottom, radius, radius);
    SetWindowRgn(hwnd, rgn, TRUE);
}

void Gui()
{   
    // Make process DPI aware and obtain main monitor scale
    ImGui_ImplWin32_EnableDpiAwareness();
    float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));
    
    // Create window for the application
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"DM Player", WS_POPUP, 100, 100, (int)(1200 * main_scale), (int)(800 * main_scale), nullptr, nullptr, wc.hInstance, nullptr);
    MakeTheWindowRound(hwnd, 30); // For GitHub Forkers Higher the radious the window will become more circlular 

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
            
        abort();
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup ImGui context
    IMGUI_CHECKVERSION();
    CreateContext();
    ImGuiIO& io = GetIO();
    io.Fonts->AddFontDefault();
    
    // Setup a font
    ImFont* MyFont = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/arial.ttf", 16.0f);

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    // Setup scaling
    ImGuiStyle& style = GetStyle();
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;

    // Set up renderers
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Now tell the GPU to give to us the loaded files that got from out ram to our vram
    static ID3D11ShaderResourceView* closebutton = LoadTexture("Assets/Window Buttons/close-window(1).png", g_pd3dDevice); // Use static to prevent memory leak 
    static ID3D11ShaderResourceView* minimizebutton = LoadTexture("Assets/Window Buttons/minimize(1).png", g_pd3dDevice);
    static ID3D11ShaderResourceView* bigbutton = LoadTexture("Assets/Window Buttons/window(1).png", g_pd3dDevice);
    static ID3D11ShaderResourceView* FirstButton = LoadTexture("Assets/Window/New Piskel.png", g_pd3dDevice);
    static ID3D11ShaderResourceView* heart = LoadTexture("Assets/Window/heart.png", g_pd3dDevice);
    static ID3D11ShaderResourceView* musicicon = LoadTexture("Assets/Window/musicalnote.png", g_pd3dDevice);
    static ID3D11ShaderResourceView* settings = LoadTexture("Assets/Window/setting.png", g_pd3dDevice);
    static ID3D11ShaderResourceView* playlist = LoadTexture("Assets/Window/pp.png", g_pd3dDevice);
    static ID3D11ShaderResourceView* plus = LoadTexture("Assets/Window/plus.png", g_pd3dDevice);
    static ID3D11ShaderResourceView* clock = LoadTexture("Assets/Window/clock.png", g_pd3dDevice);
    static ID3D11ShaderResourceView* BigNote = LoadTexture("Assets/Window/music(2).png", g_pd3dDevice);
    static ID3D11ShaderResourceView* inbox = LoadTexture("Assets/Window/inbox.png", g_pd3dDevice);
    static ID3D11ShaderResourceView* search = LoadTexture("Assets/Window/search-interface-symbol(1).png", g_pd3dDevice);
    static ID3D11ShaderResourceView* playlist1 = LoadTexture("Assets/Window/playlist.png", g_pd3dDevice);
    static ID3D11ShaderResourceView* playlist2 = LoadTexture("Assets/Window/playlist(1).png", g_pd3dDevice);
    static ID3D11ShaderResourceView* bigheart = LoadTexture("Assets/Window/heart2.png", g_pd3dDevice);

    // Main Window loop
    bool done = false;
        
    while (!done)
    {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
            
        if (done)
            break;

        // Handle window being minimized or screen locked
        if (g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
        {
            ::Sleep(10);
            continue;
        }
            
        g_SwapChainOccluded = false;

        // Handle window resize
        if (g_ResizeWidth != 430 && g_ResizeHeight != 330)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }
        
        // Change Background Color
        static ImVec4 clear_color = ImVec4(0.600f, 0.600f, 0.600f, 1.000f);
        static ImVec4 light_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        NewFrame();

        // Create A Window
        {
            Begin("DM Player", nullptr, 
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | 
            ImGuiWindowFlags_NoBringToFrontOnFocus); // Create a window and name it.
                
            SetWindowPos(ImVec2(0, 0));
            SetWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y));
            
            // Top Menu
            BeginGroup();
            {
                ImVec2 windowSize = GetWindowSize();
                
                // Add the close big and minimize button
                SetCursorPos(ImVec2(windowSize.x - 120.0f, 0.0f));
                
                PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
                PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
                
                if (Button(" ", ImVec2(38.5f, 37.8f)))
                {
                    ShowWindow(hwnd, SW_MINIMIZE);
                }
                
                SetCursorPos(ImVec2(windowSize.x - 120.0f, 0.0f));
                Image((ImTextureID)minimizebutton, ImVec2(32.0f, 32.0f));

                PopStyleColor(3);

                SetCursorPos(ImVec2(windowSize.x - 80.0f, 0.0f));
                
                PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
                PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

                if (Button("  ", ImVec2(38.5f, 37.8f)))
                {
                    ToggleMaximize(hwnd); // because ShowWindow(hwnd, SW_MAXIMIZE); did not work
                }
                
                PopStyleColor(3);

                SetCursorPos(ImVec2(windowSize.x - 70.0f, 13.0f));
                Image((ImTextureID)bigbutton, ImVec2(16.0f, 16.0f));

                SetCursorPos(ImVec2(windowSize.x - 40.0f, 0.0f));
                
                PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
                PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

                if (Button("   ", ImVec2(38.5f, 37.8f)))
                {
                    PostQuitMessage(0);
                }
                
                SetCursorPos(ImVec2(windowSize.x - 40.0f, 3.8f));
                Image((ImTextureID)closebutton, ImVec2(32.0f, 32.0f));

                PopStyleColor(3);

                SetCursorPos(ImVec2(15.0, 12.0f));
                PushFont(MyFont);
                Text("DM Player");
                PopFont();

                SetCursorPos(ImVec2(0.0f, 38.0f));
                Separator();
            }
            
            EndGroup();
            
            // Top Right Menu With Tab System in it
            BeginGroup();
            {  
                ImVec2 windowSize = GetWindowSize(); 

                float scale_x = windowSize.x / 1200.0f; 
                float scale_xyzx = windowSize.x / 1138.5f;
                float scale_xy = windowSize.x / 770.0f;
                
                float scale_y = windowSize.y / 800.0f;
                float scale_xyz = windowSize.y / 750.0f;

                PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
                PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
                PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
                PushFont(MyFont);

                if (Button("Now Playing", ImVec2(220.0f, 42.0f)))
                {
                    Tabsystem = 0; // Set it up as the starting tab
                }

                // Draw the icon on the left
                SameLine(0.0f, 0.0f);
                SetCursorPos(ImVec2(20.0f, 42.0f));
                PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.4f, 0.5f));
                Image((ImTextureID)FirstButton, ImVec2(42.0f, 42.0f));

                // Button 2
                if (Button("Library", ImVec2(220.0f, 42.0f)))
                {
                    Tabsystem = 1; // tab 2
                }

                // Draw icon
                SameLine(0.0f, 0.0f);
                SetCursorPos(ImVec2(33.0f, 102.0f));
                PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.42f, 0.5f));
                Image((ImTextureID)musicicon, ImVec2(16.0f, 16.0f));

                if (Button("Playlists", ImVec2(220.0f, 42.0f)))
                {
                    Tabsystem = 2;
                }
                
                SameLine(0.0f, 0.0f);
                SetCursorPos(ImVec2(33.0f, 148.0f));
                PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.45, 0.5f));
                Image((ImTextureID)playlist, ImVec2(16.0f, 16.0f));

                if (Button("Favourites", ImVec2(220.0f, 42.0f)))
                {
                    Tabsystem = 3;
                }
                
                SameLine(0.0f, 0.0f);
                SetCursorPos(ImVec2(33.0f, 195.0f));
                PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.4, 0.5f));
                Image((ImTextureID)heart, ImVec2(16.0f, 16.0f));

                if (Button("Settings", ImVec2(220.0f, 42.0f)))
                {
                    Tabsystem = 4;
                }

                SetCursorPos(ImVec2(33.0f, 240.0f));
                Image((ImTextureID)settings, ImVec2(16.0f, 16.0f));
                
                ImVec2 p = GetCursorScreenPos();
                GetWindowDrawList()->AddLine(ImVec2(p.x + -7.0f, p.y + 17.0f), ImVec2(p.x + 222.0f, p.y + 17.0f), 
                GetColorU32(ImGuiCol_Separator), 1.0f);

                SetCursorPos(ImVec2(40.0f * scale_x, 380.0f * scale_y));
                PushFont(NULL, 16.0f);

                Text("No playlists yet");

                PopFont();
                        
                SetCursorPos(ImVec2(20.0f * scale_x, 400.0f * scale_y));
                PushStyleColor(ImGuiCol_Text, ImVec4(0.56f, 0.56f, 0.56f, 1.0f));
                PushFont(NULL, 13.5f);

                Text("Create a playlist to get started.");
                        
                PopStyleColor();
                PopFont();

                if (g_IsMaximized == false)
                {
                    SetCursorPos(ImVec2(970.0f, 128.0f));
                }
                    
                else
                {
                    SetCursorPos(ImVec2(970.0f * scale_xyzx, 128.0f));
                }

                if (Tabsystem == 0) 
                {   
                    
                    
                    // Checking and displaying if we find the image 
                    if (takepath != "")
                    {
                        SetCursorPos(ImVec2(255.0f, 155.0f));

                        Text("image found", searchfortitle.c_str());
                    }

                    else
                    {
                        SetCursorPos(ImVec2(690.0f * scale_x, 100.0f * scale_y));
                        Image((ImTextureID)BigNote, ImVec2(64.0f, 64.0f));
                        
                        SetCursorPos(ImVec2(690.0f * scale_x, 500.0f * scale_y));
                        Image((ImTextureID)inbox, ImVec2(64.0f, 64.0f));

                        SetCursorPos(ImVec2(650.0f * scale_x, 180.0f * scale_y));
                        
                        PushFont(NULL, 28.0f);
                        
                        Text("No song playing");
    
                        SetCursorPos(ImVec2(640.0f * scale_x, 220.0f * scale_y));
                        
                        PushFont(NULL, 16.0f);
                        PushStyleColor(ImGuiCol_Text, ImVec4(0.56f, 0.56f, 0.56f, 1.0f));

                        Text("Add some songs to your library\n              to get started.");

                        PopStyleColor();

                        SetCursorPos(ImVec2(660.0f * scale_x, 580.0f * scale_y));

                        PushFont(NULL, 18.0f);

                        Text("Your playlist is empty.");

                        SetCursorPos(ImVec2(650.0f * scale_x, 610.0f * scale_y));

                        PushFont(NULL, 16.0f);
                        PushStyleColor(ImGuiCol_Text, ImVec4(0.56f, 0.56f, 0.56f, 1.0f));

                        Text("Add some songs to get started.");
                        
                        PopStyleColor();
                        PopFont();
                        PopFont();
                        PopFont();
                        PopFont();
                    }

                    GetWindowDrawList()->AddLine(ImVec2(p.x + 223.0f, p.y + 80.0f), ImVec2(p.x + 3500.0f, p.y + 80.0f), 
                    GetColorU32(ImGuiCol_Separator), 1.0f);
                    
                    GetWindowDrawList()->AddLine(ImVec2(p.x + 223.0f, p.y + 130.0f), ImVec2(p.x + 3500.0f, p.y + 130.0f), 
                    GetColorU32(ImGuiCol_Separator), 1.0f);

                    SetCursorPos(ImVec2(255.0f, 355.0f));
                    PushFont(MyFont);
                    
                    if (Button("#", ImVec2(16.0f, 16.0f)))
                    {
                        // Do something 
                    }
                    PopFont();

                    SameLine(0.0f, 30.0f * scale_x);

                    if (Button("Title", ImVec2(50.0f, 20.0f)))
                    {

                    }

                    SameLine(0.0f, 120.0f * scale_x);

                    if (Button("Artist", ImVec2(52.0f, 20.0f)))
                    {

                    }

                    SameLine(0.0f, 180.0f * scale_x);

                    if (Button("Album", ImVec2(50.0f, 20.0f)))
                    {

                    }
                    
                    // Another band aid 
                    if (g_IsMaximized == false)
                    {
                        SameLine(0.0f, 350.0f * scale_x);
                    }
                    
                    else
                    {
                        SameLine(0.0f, 350.0f * scale_xy);
                    }
                    

                    if (Button("Duration", ImVec2(80.0f, 20.0f)))
                    {
                        
                    }
                    if (g_IsMaximized == false)
                    {
                        SetCursorPos(ImVec2(1090.0f * scale_x, 356.8f));
                    }
                    
                    else 
                    {
                        SetCursorPos(ImVec2(1130.0f * scale_x, 356.8f));
                    }
                    
                    Image((ImTextureID)clock, ImVec2(16.0f, 16.0f));
                }
                
                else if (Tabsystem == 1)
                {
                    SetCursorPos(ImVec2(255.0f, 70.0f));
                    PushFont(NULL, 22.0f);

                    Text("Library");

                    PopFont();

                    SetCursorPos(ImVec2(265.0f, 120.0f));

                    if(Button("Songs", ImVec2(53.0f, 20.0f)))
                    {
                        LibraryTabSystem = 0;
                    }

                    SameLine(0.0f, 15.0f);

                    if(Button("Albums", ImVec2(55.0f, 23.0f)))
                    {
                        LibraryTabSystem = 1;
                    }

                    SameLine(0.0f, 15.0f);

                    if(Button("Foldiers", ImVec2(57.0f, 25.0f)))
                    {
                        LibraryTabSystem = 2;
                    }

                    // Band ade code till i find a solution to how to fix this 
                    if (g_IsMaximized == false)
                    {
                        SameLine(0.0f, 500.0f);
                    }
                    
                    else
                    {
                        SameLine(0.0f, 1170.0f);
                    }
                    
                    // Making a search box because ImGui is g** and it doesnt have one and expects me to make one
                   
                    SetNextItemWidth(200.0f);
                    PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(30.0f, 8.0f));
                    PushStyleColor(ImGuiCol_Text, ImVec4(0.6, 0.6, 0.6, 1.0f));
                    PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.13f, 0.13f, 0.13f, 1.0f));
                   
                    InputTextWithHint("        ", "Search library...", HoldSearch, sizeof(HoldSearch));
                    const char* items[] = {/* Add later the function with the names in order to actually search */};
                   
                    for (const char* item : items) 
                    {
                        if (strstr(item, HoldSearch) != nullptr) // Check if we have what the user is searching maybe add later a function with algorithm that can see lowercase and upercase
                        { 
                            Text("%s", item); // Very simple logic of finding the item
                        }
                    }
                    
                    if (g_IsMaximized == false)
                    {
                        SetCursorPos(ImVec2(970.0f, 128.0f));
                    }
                    
                    else
                    {
                        SetCursorPos(ImVec2(970.0f * scale_xyzx, 128.0f));
                    }

                    Image((ImTextureID)search, ImVec2(16.0f, 16.0f));
                    
                    PopStyleVar();
                    PopStyleColor(2);

                    GetWindowDrawList()->AddLine(ImVec2(p.x + 260.0f, p.y + -100.0f), ImVec2(p.x + 1820.0f, p.y + -100.0f), // first x is for left right second x is for length first and second y are for rotating
                    GetColorU32(ImGuiCol_Separator), 1.0f);

                    if (LibraryTabSystem == 0)
                    {
                        // Put this to an else statement after i make the find music statement later

                        // SetCursorPos(ImVec2(690.0f, 300.0f));

                        SetCursorPos(ImVec2(700.0f * scale_xyzx, 320.0f * scale_y));
                        
                        Image((ImTextureID)BigNote, ImVec2(64.0f, 64.0f));

                        SetCursorPos(ImVec2(690.0f * scale_x, 400.0f * scale_y));
                        PushFont(NULL, 28.0f); // Next put it 16

                        Text("You have no songs");

                        PopFont();
                        
                        SetCursorPos(ImVec2(685.0f * scale_x, 440.0f * scale_y));
                        PushStyleColor(ImGuiCol_Text, ImVec4(0.56f, 0.56f, 0.56f, 1.0f));
                        PushFont(NULL, 18.5f);

                        Text("Add some songs to your library\n             to get started.");
                        
                        PopStyleColor();
                        PopFont();

                        // Here is the end of the else statement
                    }

                    else if (LibraryTabSystem == 1)
                    {
                        // Put this to an else statement after i make the find music statement later
                        SetCursorPos(ImVec2(700.0f * scale_xyzx, 320.0f * scale_y));
                        Image((ImTextureID)BigNote, ImVec2(64.0f, 64.0f));

                        SetCursorPos(ImVec2(690.0f * scale_x, 400.0f * scale_y));
                        PushFont(NULL, 28.0f); // Next put it 16

                        Text("You have no albums");

                        PopFont();
                        
                        SetCursorPos(ImVec2(685.0f * scale_x, 440.0f * scale_y));
                        PushStyleColor(ImGuiCol_Text, ImVec4(0.56f, 0.56f, 0.56f, 1.0f));
                        PushFont(NULL, 18.5f);

                        Text("Add some songs to your library\n             to get started.");
                        
                        PopStyleColor();
                        PopFont();

                        // Here is the end of the else statement
                    }

                    else if (LibraryTabSystem == 2)
                    {
                        // Put this to an else statement after i make the find music statement later
                        SetCursorPos(ImVec2(700.0f * scale_xyzx, 320.0f * scale_y));
                        Image((ImTextureID)BigNote, ImVec2(64.0f, 64.0f));

                        SetCursorPos(ImVec2(675.0f * scale_x, 400.0f * scale_y));
                        PushFont(NULL, 28.0f); // Next put it 16

                        Text("You haven't selected a folder");

                        PopFont();
                        
                        SetCursorPos(ImVec2(680.0f * scale_x, 440.0f * scale_y));
                        PushStyleColor(ImGuiCol_Text, ImVec4(0.56f, 0.56f, 0.56f, 1.0f));
                        PushFont(NULL, 18.5f);

                        Text("Go to settings and select a folder to start.");
                        
                        PopStyleColor();
                        PopFont();

                        PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
                        PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
                        PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
                        PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
                        
                        SetCursorPos(ImVec2(710.0f * scale_xyzx, 470.0f * scale_y));

                        if (Button(" Settings ", ImVec2(70.0f, 50.0f)))
                        {
                            Tabsystem = 4;
                        }

                        PopStyleColor(3);
                        PopStyleVar();

                        // Here is the end of the else statement
                        }

                }
                
                else if (Tabsystem == 2)
                {
                    SetCursorPos(ImVec2(255.0f, 70.0f));
                    PushFont(NULL, 22.0f);

                    Text("Playlists");

                    PopFont();

                    GetWindowDrawList()->AddLine(ImVec2(p.x + 260.0f, p.y + -100.0f), ImVec2(p.x + 1820.0f, p.y + -100.0f), // first x is for left right second x is for length first and second y are for rotating
                    GetColorU32(ImGuiCol_Separator), 1.0f);

                    if (g_IsMaximized == false)
                    {
                        SetCursorPos(ImVec2(960.0f * scale_x, 120.0f));
                    }

                    else
                    {
                        SetCursorPos(ImVec2(1018.0f * scale_x, 120.0f));
                    }
                    
                    SetNextItemWidth(200.0f);
                    
                    PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(30.0f, 8.0f));
                    PushStyleColor(ImGuiCol_Text, ImVec4(0.6, 0.6, 0.6, 1.0f));
                    PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.13f, 0.13f, 0.13f, 1.0f));
                   
                    InputTextWithHint("        ", "Search playlist...", HoldSearch, sizeof(HoldSearch));
                    const char* items[] = {/* Add later the function with the names in order to actually search */};
                   
                    for (const char* item : items) 
                    {
                        if (strstr(item, HoldSearch) != nullptr) // Check if we have what the user is searching maybe add later a function with algorithm that can see lowercase and upercase
                        { 
                            Text("%s", item); // Very simple logic of finding the item
                        }
                    }

                    PopStyleVar();
                    PopStyleColor(2);
                    
                    SetCursorPos(ImVec2(700.0f * scale_xyzx, 320.0f * scale_y));
                    Image((ImTextureID)playlist1, ImVec2(64.0f, 64.0f));

                    SetCursorPos(ImVec2(700.0f * scale_x, 400.0f * scale_y));
                    PushFont(NULL, 28.0f);

                    Text("No playlists yet");

                    PopFont();
                        
                    SetCursorPos(ImVec2(687.8f * scale_x, 440.0f * scale_y));
                    PushStyleColor(ImGuiCol_Text, ImVec4(0.56f, 0.56f, 0.56f, 1.0f));
                    PushFont(NULL, 18.5f);

                    Text("Create a playlist to get started.");
                        
                    PopStyleColor();
                    PopFont();

                    if (g_IsMaximized == false)
                    {
                        SetCursorPos(ImVec2(970.0f, 128.0f));
                    }
                    
                    else
                    {
                        SetCursorPos(ImVec2(970.0f * scale_xyzx, 128.0f));
                    }

                    Image((ImTextureID)search, ImVec2(16.0f, 16.0f));
                }
                
                else if (Tabsystem == 3)
                {
                    SetCursorPos(ImVec2(255.0f, 70.0f));
                    PushFont(NULL, 22.0f);

                    Text("Favourites");

                    PopFont();

                    SetCursorPos(ImVec2(265.0f, 120.0f));

                    if(Button("Songs", ImVec2(53.0f, 20.0f)))
                    {
                        LibraryTabSystem = 0;
                    }

                    SameLine(0.0f, 15.0f);

                    if(Button("Albums", ImVec2(55.0f, 23.0f)))
                    {
                        LibraryTabSystem = 1;
                    }

                    SameLine(0.0f, 15.0f);

                    if(Button("Foldiers", ImVec2(57.0f, 25.0f)))
                    {
                        LibraryTabSystem = 2;
                    }

                    // Band ade code till i find a solution to how to fix this 
                    if (g_IsMaximized == false)
                    {
                        SameLine(0.0f, 500.0f);
                    }
                    
                    else
                    {
                        SameLine(0.0f, 1170.0f);
                    }
                   
                    SetNextItemWidth(200.0f);
                    PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(30.0f, 8.0f));
                    PushStyleColor(ImGuiCol_Text, ImVec4(0.6, 0.6, 0.6, 1.0f));
                    PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.13f, 0.13f, 0.13f, 1.0f));
                   
                    InputTextWithHint("        ", "Search library...", HoldSearch, sizeof(HoldSearch));
                    const char* items[] = {/* Add later the function with the names in order to actually search */};
                   
                    for (const char* item : items) 
                    {
                        if (strstr(item, HoldSearch) != nullptr) // Check if we have what the user is searching maybe add later a function with algorithm that can see lowercase and upercase
                        { 
                            Text("%s", item); // Very simple logic of finding the item
                        }
                    }
                    
                    if (g_IsMaximized == false)
                    {
                        SetCursorPos(ImVec2(970.0f, 128.0f));
                    }
                    
                    else
                    {
                        SetCursorPos(ImVec2(970.0f * scale_xyzx, 128.0f));
                    }

                    Image((ImTextureID)search, ImVec2(16.0f, 16.0f));
                    
                    PopStyleVar();
                    PopStyleColor(2);

                    GetWindowDrawList()->AddLine(ImVec2(p.x + 260.0f, p.y + -100.0f), ImVec2(p.x + 1820.0f, p.y + -100.0f), // first x is for left right second x is for length first and second y are for rotating
                    GetColorU32(ImGuiCol_Separator), 1.0f);

                    if (LibraryTabSystem == 0)
                    {
                        SetCursorPos(ImVec2(700.0f * scale_xyzx, 320.0f * scale_y));
                        Image((ImTextureID)bigheart, ImVec2(64.0f, 64.0f));

                        SetCursorPos(ImVec2(700.0f * scale_x, 400.0f * scale_y));
                        PushFont(NULL, 28.0f);

                        Text("No favourites yet");

                        PopFont();
                            
                        SetCursorPos(ImVec2(687.8f * scale_x, 440.0f * scale_y));
                        PushStyleColor(ImGuiCol_Text, ImVec4(0.56f, 0.56f, 0.56f, 1.0f));
                        PushFont(NULL, 18.5f);

                        Text("Add songs and albums to your favourites.");     
                        
                        PopFont();
                        PopStyleColor();
                    }

                    else if (LibraryTabSystem == 1)
                    {
                        SetCursorPos(ImVec2(700.0f * scale_xyzx, 320.0f * scale_y));
                        Image((ImTextureID)bigheart, ImVec2(64.0f, 64.0f));

                        SetCursorPos(ImVec2(700.0f * scale_x, 400.0f * scale_y));
                        PushFont(NULL, 28.0f);

                        Text("No albums yet");

                        PopFont();
                            
                        SetCursorPos(ImVec2(687.8f * scale_x, 440.0f * scale_y));
                        PushStyleColor(ImGuiCol_Text, ImVec4(0.56f, 0.56f, 0.56f, 1.0f));
                        PushFont(NULL, 18.5f);

                        Text("Add albums to your favourites.");     
                        
                        PopFont();
                        PopStyleColor();
                    }

                    else if (LibraryTabSystem == 2)
                    {
                        SetCursorPos(ImVec2(700.0f * scale_xyzx, 320.0f * scale_y));
                        Image((ImTextureID)BigNote, ImVec2(64.0f, 64.0f));

                        SetCursorPos(ImVec2(675.0f * scale_x, 400.0f * scale_y));
                        PushFont(NULL, 28.0f); // Next put it 16

                        Text("You haven't selected a folder");

                        PopFont();
                        
                        SetCursorPos(ImVec2(680.0f * scale_x, 440.0f * scale_y));
                        PushStyleColor(ImGuiCol_Text, ImVec4(0.56f, 0.56f, 0.56f, 1.0f));
                        PushFont(NULL, 18.5f);

                        Text("Go to settings and select a folder to start.");
                        
                        PopStyleColor();
                        PopFont();

                        PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
                        PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
                        PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
                        PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
                        
                        SetCursorPos(ImVec2(710.0f * scale_xyzx, 470.0f * scale_y));

                        if (Button(" Settings ", ImVec2(70.0f, 50.0f)))
                        {
                            Tabsystem = 4;
                        }

                        PopStyleColor(3);
                        PopStyleVar();
                    }

                }
                
                else if (Tabsystem == 4)
                {
                    SetCursorPos(ImVec2(255.0f, 70.0f));
                    PushFont(NULL, 22.0f);

                    Text("Settings");

                    PopFont();

                    GetWindowDrawList()->AddLine(ImVec2(p.x + 320.0f, p.y + -160.0f), ImVec2(p.x + 247.0f, p.y + -160.0f), // first x is for left right second x is for length first and second y are for rotating
                    GetColorU32(ImGuiCol_Separator), 1.0f);

                    SetCursorPos(ImVec2(255.0f, 200.0f));
                    PushFont(NULL, 25.0f);

                    Text("Set Path For music:");

                    PopFont();

                    //InputText("(Select Ur Runing Process)", IAMTIRED); Add it when i make the searching function
                    
                    PushFont(NULL, 22.0f);

                    SetCursorPos(ImVec2(255.0f, 300.0f));

                    if (Checkbox("Dark mode", &idk))
                    {
                        if (idk)
                        {
                            clear_color = ImVec4(0.600f, 0.600f, 0.600f, 1.000f);
                        }

                        else
                        {
                            clear_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                        }
                        
                    }

                    else
                    {
                        
                    }

                    PopFont();
                }

                PopStyleColor(3);
                PopStyleVar(5);
                PopFont();
            
            }
        }

        EndGroup();

        // Bottom Right Menu
        BeginGroup();
        {
            SetCursorPos(ImVec2(25.0f, 300.0f));
            PushFont(MyFont);
            Text("Playlists");
            
            PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
            PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));

            SetCursorPos(ImVec2(190.0f, 297.0f));
            if (ImageButton("plus", (ImTextureID)plus, ImVec2(16.0f, 16.0f)))
            {
                // Do something
            }

            PopStyleColor(3);
            PopFont();
        }
        
        EndGroup();
        
            SetCursorPos(ImVec2(230.0f, 38.0f));

            ImVec2 pos = GetCursorScreenPos();

            ImDrawList* draw_list = GetWindowDrawList();

            ImU32 col = GetColorU32(ImGuiCol_Separator);

            draw_list->AddLine(
                ImVec2(pos.x, pos.y),
                ImVec2(pos.x, pos.y + GetWindowSize().y - 38.0f),
                col,
                1.0f
            );


        End();
        
        
        // Rendering
        Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(GetDrawData());

        // Present
        HRESULT hr = g_pSwapChain->Present(1, 0);   // Present with vsync
        g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
    }
        
    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

}
    
bool CreateDeviceD3D(HWND hWnd)
{
    // This is a basic setup. Optimally could use e.g. DXGI_SWAP_EFFECT_FLIP_DISCARD and handle fullscreen mode differently. See #8979 for suggestions.
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
         res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam);
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;

    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;

    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;

    case WM_NCHITTEST:
    {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
            return true;

        // Getting the mouse possision 
        float mouseX = (float)(short)LOWORD(lParam);
        float mouseY = (float)(short)HIWORD(lParam);

        // Convert to inside window cordinates
        POINT pt;
        pt.x = (LONG)mouseX;
        pt.y = (LONG)mouseY;
        ScreenToClient(hWnd, &pt);

        // Make the top earia draggable 
        float topbar = 40.0f;

        // Check if mouse is on top area and if it is drag window
        if (pt.y >= 0 && pt.y <= topbar)
        {
            // Make sure u cannot drag on the - o x buttons
            if (pt.x > 1050)
            {
                return HTCLIENT;
            }

            return HTCAPTION;   // Make it actually draggable 
        }

        return HTCLIENT;
    }

    }

    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

void ToggleMaximize(HWND hwnd)
{
    if (g_IsMaximized)
    {
        // Restore
        SetWindowPos(hwnd, nullptr, g_NormalRect.left, g_NormalRect.top, g_NormalRect.right - g_NormalRect.left, g_NormalRect.bottom - g_NormalRect.top, SWP_NOZORDER | SWP_NOACTIVATE);
            
        MakeTheWindowRound(hwnd, 30);
        g_IsMaximized = false;
    }
    else
    {
        // Save current position and size
        GetWindowRect(hwnd, &g_NormalRect);
        
        // Get screen work area (without taskbar)
        MONITORINFO mi = { sizeof(MONITORINFO) };
        GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), &mi);
        
        SetWindowPos(hwnd, nullptr, mi.rcWork.left, mi.rcWork.top, mi.rcWork.right - mi.rcWork.left, mi.rcWork.bottom - mi.rcWork.top, SWP_NOZORDER | SWP_NOACTIVATE);
            
        MakeTheWindowRound(hwnd, 0);
        g_IsMaximized = true;
    }
}

int main()
{
    Gui();

    return 0;
}
