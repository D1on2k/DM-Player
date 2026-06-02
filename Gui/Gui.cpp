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

#define STB_IMAGE_IMPLEMENTATION

#include "Gui.h"
#include "stb_image.h"

using namespace std;
using namespace ImGui;

ID3D11Device* g_pd3dDevice = nullptr;
ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
IDXGISwapChain* g_pSwapChain = nullptr;
UINT g_ResizeWidth = 0, g_ResizeHeight = 0;
ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

// Declear them here because compiler hates me 
bool songsLoaded = false;
bool idk = true;
bool scan = false;
bool g_SwapChainOccluded = false;
bool test = true;
bool g_IsMaximized = false;
bool pathChanged = false;
bool pause = false;
bool isitEmpty = true;
static bool image = false;

struct Playlist 
{
    std::string name;
    std::vector<int> songokok;
};

RECT g_WindowRectWhenNormal = {0};
RECT g_NormalRect = { 0, 0, 0, 0 };

string previousPath = "";

int Tabsystem = 0;
int LibraryTabSystem = 0;
int selectedIndex = -1;
int ActiveFolderViewIndex = -1;
static int plalistselectedindex = -1;

char HoldSearch[128] = "";
char pathBuffer[512] = {};
char IAMTIRED[256] = "";

std::vector<SongDisplay> songlist;
std::vector<std::string> songtitles;
std::vector<FolderDisplay> folderTabList;
static std::vector<Playlist> g_Playlists;

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
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"DM Player", WS_POPUP | WS_MINIMIZEBOX | WS_VISIBLE, 100, 100, (int)(1200 * main_scale), (int)(800 * main_scale), nullptr, nullptr, wc.hInstance, nullptr);
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

    // Initialize my backend
    playerinitilize();

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
    static ID3D11ShaderResourceView* albemt = LoadTexture("Assets/Window/AlbumEmpty.png", g_pd3dDevice);
    static ID3D11ShaderResourceView* volume2 = LoadTexture("Assets/Window/volume-up(1).png", g_pd3dDevice);
    static ID3D11ShaderResourceView* pausems = LoadTexture("Assets/Window/pause.png", g_pd3dDevice);
    static ID3D11ShaderResourceView* playsng = LoadTexture("Assets/Window/play-button-arrowhead.png", g_pd3dDevice);
    static ID3D11ShaderResourceView* lastsong = LoadTexture("Assets/Window/rewind.png", g_pd3dDevice);
    static ID3D11ShaderResourceView* next = LoadTexture("Assets/Window/forward.png", g_pd3dDevice);
    static ID3D11ShaderResourceView* loop = LoadTexture("Assets/Window/loop.png", g_pd3dDevice);
    static ID3D11ShaderResourceView* loop1 = LoadTexture("Assets/Window/loop(1).png", g_pd3dDevice);
    static ID3D11ShaderResourceView* albumTexture = nullptr;

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

        // make it that was so if the song finishes to play the next will start btw if i kept it inside the g_swapchain it would just not work 
        if (playerifsongjustfinished())
        {
            if (Tabsystem == 2 && plalistselectedindex != -1)
            {
                auto& activePlaylist = g_Playlists[plalistselectedindex];
                if (!activePlaylist.songokok.empty())
                {
                    int currentPlaylistPos = -1;
                    for (size_t i = 0; i < activePlaylist.songokok.size(); i++)
                    {
                        if (activePlaylist.songokok[i] == selectedIndex)
                        {
                            currentPlaylistPos = (int)i;
                            break;
                        }
                    }

                    if (currentPlaylistPos != -1)
                    {
                        int nextPlaylistPos = (currentPlaylistPos + 1) % (int)activePlaylist.songokok.size();
                        selectedIndex = activePlaylist.songokok[nextPlaylistPos];
                    }
                    else
                    {
                        selectedIndex = activePlaylist.songokok[0];
                    }

                    Sleep(60);

                    if (playerplay(songlist[selectedIndex].path))
                    {
                        pause = false;
                    }
                    else
                    {
                        pause = true;
                    }
                }
            }
                    
            else
            {
                if (!songlist.empty())
                {
                    selectedIndex = (selectedIndex + 1) % (int)songlist.size();
                            
                    Sleep(60);  
                            
                    if (playerplay(songlist[selectedIndex].path))
                    {
                        pause = false;
                    }
                    else
                    {
                        pause = true; 
                    }
                }
            }
        }
        
        // Handle window being minimized or screen locked
        if (g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
        {
            ::Sleep(10);
            continue;
        }
            
        g_SwapChainOccluded = false;

        // Handle window resize
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
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
                float scale_xyzc = windowSize.x / 1050.0f;
                
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

                if (g_IsMaximized == false)
                {
                    SetCursorPos(ImVec2(970.0f, 128.0f));
                }
                    
                else
                {
                    SetCursorPos(ImVec2(970.0f * scale_xyzx, 128.0f));
                }

                SetCursorPos(ImVec2(255.0f, 420.0f));
                PushFont(MyFont);

                ldsngsifneeded();

                PopFont();

                SetCursorPos(ImVec2(65.0f, 750.0f * scale_y));
                if (selectedIndex >= 0 && selectedIndex < (int)songlist.size())
                {
                    std::string songName = songlist[selectedIndex].title;
                    float boxWidth = 200.0f;
                    
                    PushClipRect(ImVec2(25.0f, 735.0f), ImVec2(25.0f + boxWidth, 765.0f), true);
                    float nameWidth = CalcTextSize(songName.c_str()).x;
                    
                    if (nameWidth > boxWidth)
                    {
                        float scrollSpeed = 25.0f; // enought to read but show everything 
                        float waitTime = 3.0f; // 3 sec seems like the sweep stop for most including me 
                        float scrollDistance = nameWidth - boxWidth;
                        float onewayTime = (scrollDistance / scrollSpeed) + waitTime;
                        float timer = fmod((float)ImGui::GetTime(), onewayTime * 2.0f);
                        float scrollOffset = 0.0f;
                        
                        if (timer < waitTime)
                            scrollOffset = 0.0f;
                        
                        else if (timer < waitTime + onewayTime)
                            scrollOffset = ImMin((timer - waitTime) * scrollSpeed, scrollDistance);
                        
                        else if (timer < waitTime * 2.0f + onewayTime)
                            scrollOffset = scrollDistance;
                        
                        else
                            scrollOffset = ImMax(scrollDistance - (timer - waitTime * 2.0f - onewayTime) * scrollSpeed, 0.0f);
                        
                        SetCursorPos(ImVec2(10.0f - scrollOffset, 750.0f));
                    }
                    
                    TextUnformatted(songName.c_str());
                    PopClipRect();
                }
                
                else
                {
                    TextUnformatted("No song playing");
                }
                
                // note from yesterday this is the float bar i will use tommorow 
                // sliderfloat("name", &volume, 0.0f, 1.0f);
                // setvolume(volume)
                                
                // note from the tommorow me which was the yesterday comment: Thank you old self
                SetCursorPos(ImVec2(0.0f, 715.0f * scale_y));
                    
                Separator();

                SetCursorPos(ImVec2(840.0f * scale_xyzc,750.0f * scale_y));
                    
                Image((ImTextureID)volume2, ImVec2(20.0f, 20.0f));

                SetCursorPos(ImVec2(200.0f * scale_xy, 750.0f * scale_y));

                if (ImageButton("##playlastsong",(ImTextureID)lastsong, ImVec2(16.0f, 16.0f)))
                {
                    if (!songlist.empty())
                    {
                           if (selectedIndex > 0)
                           {
                            selectedIndex--;
                        }

                        else
                        {
                            selectedIndex = (int)songlist.size() - 1; // make it so if it reaches the last just make it loop
                        }

                        playerplay(songlist[selectedIndex].path);
                        pause = false;
                    }
                }

                SameLine(0.0f, 15.0f);

                // big brain move so if the user wants to pause or play we have to make the icon change dynamically too so i came to this solve

                ImTextureID bigbrainmove;

                if (pause)
                {
                    bigbrainmove = (ImTextureID)playsng;
                }

                else
                {
                    bigbrainmove = (ImTextureID)pausems;
                }

                if (ImageButton("##play", bigbrainmove, ImVec2(16.0f, 16.0f))) // found on stack overflow i can use ## instade of just adding spaces
                {
                    if (selectedIndex >= 0 && selectedIndex < songlist.size())
                    {
                        if (pause)
                        {
                            playerresume();
                            pause = false;
                        }

                        else
                        {
                            playerpause();
                            pause = true;
                        }
                    }

                    else if (!songlist.empty())
                    {
                        selectedIndex = 0;
                        playerplay(songlist[0].path);
                        pause = false;
                    }
                }

                SameLine(0.0f, 14.0f);

                if (ImageButton("##nextbutton", (ImTextureID)next, ImVec2(16.0f, 16.0f)))
                {
                    if (!songlist.empty())
                    {
                        if (selectedIndex < (int)songlist.size() - 1)
                        {
                            selectedIndex++;
                        }

                        else
                        {
                            selectedIndex = 0;
                        }

                        playerplay(songlist[selectedIndex].path);
                        pause = false;
                    }
                }
                    
                SameLine(0.0f, 10.0f);

                ImTextureID okok;
                static bool loopingorno = false;

                if (loopingorno)
                {
                    okok = (ImTextureID)loop1;
                }
                    
                else
                {
                    okok = (ImTextureID)loop;
                }

                if (ImageButton("##loopbutton", (ImTextureID)okok, ImVec2(18.0f, 18.0f)))
                {   
                    loopingorno = !loopingorno; // make it change the boolean
                    playerrepeat();
                }
                    
                float currentTime = 0.0f;
                float totalTime = 0.0f;
                bool isSongPlaying = playersongprosomething(currentTime, totalTime);

                SetCursorPos(ImVec2(300.0f * scale_xy, 750.0f * scale_y));
                Text("%s", FormatTime(currentTime).c_str());

                SameLine(0.0f, 8.0f);
                    
                SetNextItemWidth(300.0f);

                // Used LLM to help me here
                //float currentTime = 0.0f;
                //float totalTime = 0.0f;

                // call the function so we dont lose time
                //bool isSongPlaying = playersongprosomething(currentTime, totalTime);

                if (isSongPlaying || g_playit != nullptr)
                {
                    static float staticProgress = 0.0f;
                    static bool isDragging = false;

                    // if the user is not draging the bar we will make the bar move normally
                    if (!isDragging)
                    {
                        staticProgress = (totalTime > 0.0f) ? (currentTime / totalTime) : 0.0f;
                    }

                    SetCursorPos(ImVec2(300.0f * scale_xy, 750.0f * scale_y));
                        
                    // make it show the draged time
                    if (isDragging)
                    {
                        Text("%s", FormatTime(staticProgress * totalTime).c_str());
                    }
                    else
                    {
                        Text("%s", FormatTime(currentTime).c_str());
                    }

                    SameLine(0.0f, 8.0f);

                    SetNextItemWidth(300.0f);
                    PushStyleVar(ImGuiStyleVar_GrabRounding, 999.0f); // make it as round as i can
                    PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 2.0f));
                    PushStyleVar(ImGuiStyleVar_GrabMinSize, 26.0f); // thank god vs code knows every style var i wouldnt go searching for it online 
                        
                    PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.30f, 0.30f, 0.30f, 1.0f));
                    PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.30f, 0.30f, 0.30f, 1.0f));
                    PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.30f, 0.30f, 0.30f, 1.0f));
                    PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.45f, 0.45f, 0.45f, 1.0f));
                    PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.45f, 0.45f, 0.45f, 1.0f));
                        
                    // this returns true while the user is sliding
                    if (SliderFloat("##findtime", &staticProgress, 0.0f, 1.0f, ""))
                    {
                        isDragging = true;
                    }

                    if (IsItemDeactivatedAfterEdit())
                    {
                        playerfindtime(staticProgress);
                        isDragging = false; 
                    }

                    SameLine(0.0f, 8.0f);
                    Text("%s", FormatTime(totalTime).c_str());

                    PopStyleVar(3);
                    PopStyleColor(5);
                }
                    
                else
                {
                    SetCursorPos(ImVec2(300.0f * scale_xy, 750.0f * scale_y));
                    Text("00:00");

                    SameLine(0.0f, 8.0f);
                    SetNextItemWidth(300.0f);
                        
                    float dummy = 0.0f;
                    SliderFloat("##idk", &dummy, 0.0f, 1.0f, "");
                        
                    SameLine(0.0f, 8.0f);
                    Text("00:00");
                }

                SetCursorPos(ImVec2(860.0f * scale_xyzc, 750.0f * scale_y));
                    
                /*
                My research notes:
                    
                ImGuiCol_SliderGrab / Color of the slider's grab handle
                ImGuiCol_SliderGrabActive / Color of the grab handle when active
                ImGuiStyle::GrabMinSize / Minimum size of the grab handle
                ImGuiStyle::FramePadding / Padding around the slider
                    
                // I made this but i dont know exactly how to use it
                //IMGUI_API bool grabthingrounding(const char* label, float* p_value, float v_min, float v_max, float v_step=50.f);
                */

                PushStyleVar(ImGuiStyleVar_GrabRounding, 999.0f); // make it as round as i can
                PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 2.0f));
                PushStyleVar(ImGuiStyleVar_GrabMinSize, 26.0f); // thank god vs code knows every style var i wouldnt go searching for it online 
                    
                PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.30f, 0.30f, 0.30f, 1.0f));
                PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.30f, 0.30f, 0.30f, 1.0f));
                PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.30f, 0.30f, 0.30f, 1.0f));
                PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.45f, 0.45f, 0.45f, 1.0f));
                PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.45f, 0.45f, 0.45f, 1.0f));
                    
                SetNextItemWidth(200.0f);
                if (SliderFloat("", &volume, 0.0f, 1.0f, "")) // will add an icon maybe not ImGuiSliderFlags_NoRoundToFormat
                {
                    setvolume(volume);
                }
                    
                PopStyleVar(3);
                PopStyleColor(5);

                if (Tabsystem == 0) 
                {   

                    // Clear the window with a rectangle 
                    //GetWindowDrawList()->AddRectFilled(ImVec2(260.0f, 70.0f), ImVec2(460.0f, 270.0f), IM_COL32(0, 0, 0, 255));
                    if (!scan || WStringToString(FoldierPath) != searchpathformusic)
                    {
                        if (albumTexture)
                        {
                            albumTexture->Release();
                            albumTexture = nullptr;
                        }

                        findimages();
                        scan = true;

                        // thought of doing it the easy way which is drawing the
                        // album every frame but its very unoptmized so i did the static method

                        if (!takepath.empty())
                        {
                            albumTexture = LoadTexture(takepath.c_str(), g_pd3dDevice);
                            image = true;
                        }

                        else
                        {
                            image = false;
                        }
                    }
                    
                    if (image && albumTexture != nullptr)
                    {
                        SetCursorPos(ImVec2(260.0f, 70.0f));
                        Image((ImTextureID)albumTexture, ImVec2(200.0f, 200.0f));
                    }
                    
                    else if (!songlist.empty())
                    {
                        SetCursorPos(ImVec2(260.0f, 70.0f));
                        GetWindowDrawList()->AddRectFilled(ImVec2(260.0f, 70.0f), ImVec2(460.0f, 270.0f), IM_COL32(61, 61, 61, 255));
                        Image((ImTextureID)albemt, ImVec2(200.0f, 200.0f));
                    }

                    
                    if (!songlist.empty())
                    {   
                        // Used help from LLM here I couldn't find a way to put the name under name album under album etc but i did tweak it a bit my self.
                        ImVec2 ws = GetWindowSize();
                        float sx = ws.x / 1200.0f;

                        float tableStartY = -320.0f; // height of the thing 
                        float tableEndY   = ws.y - 800.0f; // bottom padding
                        float availableH  = tableEndY - tableStartY - 0.0f; // 30 = header row height

                        int songCount    = (int)songlist.size();
                        float rowHeight  = availableH / (float)songCount;

                        //if (rowHeight < 10.0f) rowHeight = 10.0f; // minimum so text isnt invisible
                        if (rowHeight > 0.0f) rowHeight = 0.0f; // never taller than this 20

                        SetCursorPos(ImVec2(250.0f, 355.0f));

                        PushStyleColor(ImGuiCol_TableHeaderBg,    ImVec4(0, 0, 0, 0));
                        PushStyleColor(ImGuiCol_TableBorderLight, ImVec4(0, 0, 0, 0));
                        PushStyleColor(ImGuiCol_TableBorderStrong,ImVec4(0, 0, 0, 0));
                        PushStyleColor(ImGuiCol_HeaderHovered,    ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
                        PushStyleColor(ImGuiCol_HeaderActive,     ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
                        PushStyleColor(ImGuiCol_ButtonActive,     ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

                        ImGuiTableFlags flags =
                            ImGuiTableFlags_SizingFixedFit |
                            ImGuiTableFlags_BordersInnerH  |
                            ImGuiTableFlags_ScrollY |
                            ImGuiTableFlags_NoSavedSettings;

                        float tableW = ws.x - 185.0f;

                        if (BeginTable("SongTable", 5, flags, ImVec2(tableW, availableH + 30.0f)))
                        {
                            TableSetupColumn("#",        ImGuiTableColumnFlags_WidthFixed, 40.0f);
                            TableSetupColumn("Title",    ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultHide, 240.0f * sx);
                            TableSetupColumn("Artist",   ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultHide, 200.0f * sx);
                            TableSetupColumn("Album",    ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultHide, 200.0f * sx);
                            TableSetupColumn("Duration", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultHide, 80.0f  * sx);

                            // This is what keeps the header fixed while scrolling songs
                            TableSetupScrollFreeze(0, 1);

                            TableHeadersRow();

                            // Space between header and first song row
                            TableNextRow(ImGuiTableRowFlags_None, 23.0f);

                            std::string artist, album;
                            idk12123(foldiername, artist, album);

                            PushFont(MyFont);

                            for (int i = 0; i < songCount; i++)
                            {
                                const auto& song = songlist[i];
                                TableNextRow(ImGuiTableRowFlags_None, rowHeight);

                                TableSetColumnIndex(0);
                                Text("%02d", i + 1);

                                TableSetColumnIndex(1);
                                
                                std::string selID = "##sel" + std::to_string(i);
                                
                                if (ImGui::Selectable(selID.c_str(), selectedIndex == i, ImGuiSelectableFlags_SpanAllColumns, ImVec2(0, rowHeight)))
                                {
                                    selectedIndex = i;
                                    
                                    if (!song.path.empty())
                                    {
                                        if (playerplay(song.path))
                                        {
                                            printf("idk: %s\n", song.title.c_str());
                                        }
                                        
                                        else
                                        {
                                            printf("idk1%s\n", song.title.c_str());
                                        }
                                    }
                                }
                                
                                SameLine();
                                TextUnformatted(song.title.c_str());

                                TableSetColumnIndex(2);
                                TextUnformatted(artist.c_str());

                                TableSetColumnIndex(3);
                                TextUnformatted(album.c_str());

                                TableSetColumnIndex(4);
                                Text(""); 
                            }

                            PopStyleColor(6);
                            PopFont();
                            EndTable();
                        }
                    }
                
                    else
                    {
                        SetCursorPos(ImVec2(255.0f, 355.0f));
                        PushFont(MyFont);
                        
                        if (Button("#", ImVec2(16.0f, 16.0f)))
                        {
                            // Do something 
                        }
                        PopFont();

                        SameLine(0.0f, 70.0f * scale_x);

                        if (Button("Title", ImVec2(50.0f, 20.0f)))
                        {

                        }

                        SameLine(0.0f, 150.0f * scale_x);

                        if (Button("Artist", ImVec2(52.0f, 20.0f)))
                        {

                        }

                        SameLine(0.0f, 160.0f * scale_x);

                        if (Button("Album", ImVec2(50.0f, 20.0f)))
                        {

                        }
                        
                        // Another band aid 
                        if (g_IsMaximized == false)
                        {
                            SameLine(0.0f, 300.0f * scale_x);
                        }
                        
                        else
                        {
                            SameLine(0.0f, 320.0f * scale_xy);
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

                    if (albumTexture != nullptr)
                    {
                        SetCursorPos(ImVec2(260.0f, 70.0f));

                        Image((ImTextureID)albumTexture, ImVec2(200.0f, 200.0f));
                    }
                    
                    if (!songlist.empty())
                    {   
                        std::string artist, album;
                        idk12123(foldiername, artist, album);

                        SetCursorPos(ImVec2(494.0f, 170.0f));
                        PushFont(NULL, 24.0f);
                        Text("%s", album.c_str()); // searchfortitle.c_str() is for song name
                        PopFont();

                        PushFont(NULL, 16.0f);
                        SetCursorPos(ImVec2(500.0f, 200.0f));
                        Text("%s", artist.c_str());
                        PopFont();

                        
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
                        PopFont();
                        
                        SetCursorPos(ImVec2(640.0f * scale_x, 220.0f * scale_y));
                        
                        PushFont(NULL, 16.0f);
                        PushStyleColor(ImGuiCol_Text, ImVec4(0.56f, 0.56f, 0.56f, 1.0f));

                        Text("Add some songs to your library\n              to get started.");
                        PopFont();

                        PopStyleColor();

                        SetCursorPos(ImVec2(660.0f * scale_x, 580.0f * scale_y));

                        PushFont(NULL, 18.0f);

                        Text("Your playlist is empty.");
                        PopFont();

                        SetCursorPos(ImVec2(650.0f * scale_x, 610.0f * scale_y));

                        PushFont(NULL, 16.0f);
                        PushStyleColor(ImGuiCol_Text, ImVec4(0.56f, 0.56f, 0.56f, 1.0f));

                        Text("Add some songs to get started.");
                        PopFont();

                        PopStyleColor();
                    }

                    GetWindowDrawList()->AddLine(ImVec2(p.x + 223.0f, p.y + 80.0f), ImVec2(p.x + 3500.0f, p.y + 80.0f), 
                    GetColorU32(ImGuiCol_Separator), 1.0f);
                    
                    GetWindowDrawList()->AddLine(ImVec2(p.x + 223.0f, p.y + 130.0f), ImVec2(p.x + 3500.0f, p.y + 130.0f), 
                    GetColorU32(ImGuiCol_Separator), 1.0f);
                    

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
                        ActiveFolderViewIndex = -1;
                    }

                    SameLine(0.0f, 15.0f);

                    if(Button("Albums", ImVec2(55.0f, 23.0f)))
                    {
                        LibraryTabSystem = 1;
                        ActiveFolderViewIndex = -1;
                    }

                    SameLine(0.0f, 15.0f);

                    if(Button("Foldiers", ImVec2(57.0f, 25.0f)))
                    {
                        LibraryTabSystem = 2;
                        ActiveFolderViewIndex = -1;
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
                        // Put this to an else statement after i make the find music statement later future me i did it.
                        if (isitEmpty)
                        {   

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
                            
                             // Here is the end of the else statement (i will add it pretty soon its almost copy pasting my past code) yea it was copy paste
                        }
                        
                        else
                        {
                            if (g_Playlists.empty())
                            {
                                SetCursorPos(ImVec2(700.0f * scale_xyzx, 320.0f * scale_y));
                                Image((ImTextureID)BigNote, ImVec2(64.0f, 64.0f));

                                SetCursorPos(ImVec2(690.0f * scale_x, 400.0f * scale_y));
                                PushFont(NULL, 28.0f);
                                Text("You have no playlists");
                                PopFont();
                            }
                            
                            else
                            {   
                                // Add soon
                            }
                        }
                        // Here is the end of the else statement
                    }
                    
                    else if (LibraryTabSystem == 1)
                    {
                        ImVec2 windowSize = GetWindowSize();
                        
                        if (ActiveFolderViewIndex != -1)
                        {
                            SetCursorPos(ImVec2(270.0f, 170.0f));
                            if (Button("< Go Back", ImVec2(180.0f, 25.0f)))
                            {
                                ActiveFolderViewIndex = -1;
                            }
                            
                            if (ActiveFolderViewIndex != -1)
                            {
                                const auto& currentFolder = folderTabList[ActiveFolderViewIndex];
                                SetCursorPos(ImVec2(270.0f, 205.0f));
                                Text("Album: %s", currentFolder.folderName.c_str());
                                
                                SetCursorPos(ImVec2(270.0f, 240.0f));
                                if (BeginTable("##Foliertracktable", 1, ImGuiTableFlags_ScrollY, ImVec2(900.0f * scale_x, 470.0f * scale_y)))
                                {
                                    TableSetupColumn("Song Title", ImGuiTableColumnFlags_WidthStretch);
                                    TableHeadersRow();
                                    
                                    for (size_t k = 0; k < currentFolder.songIndices.size(); k++)
                                    {
                                        int globalSongIdx = currentFolder.songIndices[k];
                                        if (globalSongIdx < 0 || globalSongIdx >= (int)songlist.size()) continue;
                                        const auto& track = songlist[globalSongIdx];
                                        TableNextRow(ImGuiTableRowFlags_None, 28.0f);
                                        TableSetColumnIndex(0);
                                        BeginGroup();
                                        
                                        float currentX = GetCursorPosX();
                                        SetCursorPosX(currentX + 8.0f * scale_x);
                                        TextDisabled("%d", (int)(k + 1));
                                        SameLine();
                                        SetCursorPosX(currentX + 35.0f * scale_x);
                                        
                                        std::string cleanTitle = track.title;
                                        size_t hyphenPos = cleanTitle.find('-');
                                        if (hyphenPos != std::string::npos && hyphenPos < 5)
                                        {
                                            size_t firstLetterPos = cleanTitle.find_first_not_of("0123456789 ", hyphenPos + 1);
                                            if (firstLetterPos != std::string::npos)
                                                cleanTitle = cleanTitle.substr(firstLetterPos);
                                        }
                                        
                                        std::string label = cleanTitle + "##fldTrack" + std::to_string(k);
                                        std::string popupId = "##trackPopup" + std::to_string(k);
                                        
                                        if (Selectable(label.c_str(), selectedIndex == globalSongIdx, ImGuiSelectableFlags_SpanAllColumns))
                                        {
                                            selectedIndex = globalSongIdx;
                                            playerplay(track.path);
                                            pause = false;
                                        }
                                        
                                        if (IsItemClicked(ImGuiMouseButton_Right))
                                            OpenPopup(popupId.c_str());
                                        
                                        EndGroup();
                                        Dummy(ImVec2(0.0f, 4.0f * scale_y));
                                        Separator();
                                        Dummy(ImVec2(0.0f, 4.0f * scale_y));
                                        
                                        PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
                                        if (BeginPopup(popupId.c_str()))
                                        {
                                            if (!g_Playlists.empty())
                                            {
                                                if (BeginMenu("Add to Playlist"))
                                                {
                                                    for (size_t p = 0; p < g_Playlists.size(); p++)
                                                    {
                                                        if (MenuItem(g_Playlists[p].name.c_str()))
                                                        {
                                                            auto& vec = g_Playlists[p].songokok;
                                                            if (std::find(vec.begin(), vec.end(), globalSongIdx) == vec.end())
                                                                vec.push_back(globalSongIdx);
                                                        }
                                                    }
                                                    ImGui::EndMenu();
                                                }
                                            }
                                            else
                                            {
                                                TextDisabled("Create a playlist to get started");
                                            }
                                            EndPopup();
                                        }
                                        PopStyleVar();
                                    }
                                    EndTable();
                                }
                            }
                        }
                        else
                        {
                            if (folderTabList.empty())
                            {
                                SetCursorPos(ImVec2(700.0f * scale_xyzx, 320.0f * scale_y));
                                Image((ImTextureID)BigNote, ImVec2(64.0f, 64.0f));
                                SetCursorPos(ImVec2(690.0f * scale_x, 400.0f * scale_y));
                                PushFont(NULL, 28.0f);
                                Text("You have no albums");
                                PopFont();
                                SetCursorPos(ImVec2(685.0f * scale_x, 440.0f * scale_y));
                                PushStyleColor(ImGuiCol_Text, ImVec4(0.56f, 0.56f, 0.56f, 1.0f));
                                PushFont(NULL, 18.5f);
                                Text("Add some songs to your library\nto get started.");
                                PopStyleColor();
                                PopFont();
                            }
                            else
                            {
                                SetCursorPos(ImVec2(270.0f, 170.0f));
                                TextDisabled("Albums");
                                
                                SetCursorPos(ImVec2(270.0f, 210.0f));
                                float childWidth = windowSize.x - (270.0f * scale_x) - (30.0f * scale_x);
                                float childHeight = windowSize.y - 300.0f - 20.0f;
                                
                                if (BeginChild("##Foldiergrid", ImVec2(childWidth, childHeight), false, ImGuiWindowFlags_NoBackground))
                                {
                                    float cardWidth = 130.0f * scale_x;
                                    float cardHeight = 210.0f * scale_y;
                                    float padding = 25.0f * scale_x;
                                    float startX = 0.0f;
                                    float startY = 0.0f;
                                    float currentX = startX;
                                    float currentY = startY;

                                    for (size_t f = 0; f < folderTabList.size(); f++)
                                    {
                                        SetCursorPos(ImVec2(currentX, currentY));
                                        BeginGroup();

                                        if (!folderTabList[f].imgSearched)
                                        {
                                            folderTabList[f].imgSearched = true;

                                            std::filesystem::path basePath = FoldierPath;
                                            std::filesystem::path albumFullPath = folderTabList[f].fullFolderPath;
                                            
                                            std::vector<std::string> coverPaths = searchinfoldier(albumFullPath.string());

                                            if (!coverPaths.empty())
                                            {
                                                folderTabList[f].folderImg = LoadTexture(coverPaths[0].c_str(), g_pd3dDevice);
                                            }
                                        }

                                        if (folderTabList[f].folderImg != nullptr)
                                        {
                                            ImGui::Image((ImTextureID)folderTabList[f].folderImg, ImVec2(cardWidth, cardWidth));
                                        }
                                        else
                                        {
                                            ImDrawList* drawList = GetWindowDrawList();
                                            ImVec2 pMin = GetCursorScreenPos();
                                            ImVec2 pMax = ImVec2(pMin.x + cardWidth, pMin.y + cardWidth);
                                            drawList->AddRectFilled(pMin, pMax, IM_COL32(45, 45, 48, 255), 6.0f);
                                            InvisibleButton(("##imgPlaceholder" + std::to_string(f)).c_str(), ImVec2(cardWidth, cardWidth));
                                        }

                                        PushTextWrapPos(GetCursorPosX() + cardWidth);
                                        TextUnformatted(folderTabList[f].folderName.c_str());
                                        TextDisabled("%d Songs", (int)folderTabList[f].songIndices.size());
                                        PopTextWrapPos();

                                        EndGroup();

                                        if (IsItemClicked())
                                        {
                                            ActiveFolderViewIndex = (int)f;
                                        }

                                        currentX += cardWidth + padding;
                                        if (currentX + cardWidth > childWidth - 20.0f * scale_x)
                                        {
                                            currentX = startX;
                                            currentY += cardHeight + padding;
                                        }
                                    }
                                }
                                EndChild();
                            }
                        }
                    }

                    else if (LibraryTabSystem == 2) 
                    {   
                        if (folderTabList.empty())
                        {
                            // Big thanks to GitHub it became clutch it provided me with my old good looking code i am still in a confused state with the scale_x,y etc but it preserved my code so i could just copy paste the old polished part
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

                        else if (LibraryTabSystem == 2)
                        {
                            SetCursorPos(ImVec2(255.0f, 70.0f));
                            PushFont(NULL, 22.0f);
                            Text("Folders");
                            PopFont();

                            if (folderTabList.empty())
                            {
                                SetCursorPos(ImVec2(700.0f * scale_xyzx, 320.0f * scale_y));
                                Image((ImTextureID)BigNote, ImVec2(64.0f, 64.0f));

                                SetCursorPos(ImVec2(675.0f * scale_x, 400.0f * scale_y));
                                PushFont(NULL, 28.0f);
                                Text("You haven't selected a folder");
                                PopFont();

                                SetCursorPos(ImVec2(680.0f * scale_x, 440.0f * scale_y));
                                PushStyleColor(ImGuiCol_Text, ImVec4(0.56f, 0.56f, 0.56f, 1.0f));
                                PushFont(NULL, 18.5f);
                                Text("Go to settings and select a folder to start.");
                                PopStyleColor();
                                PopFont();

                                SetCursorPos(ImVec2(710.0f * scale_xyzx, 470.0f * scale_y));
                                if (Button(" Settings ", ImVec2(70.0f, 50.0f)))
                                {
                                    Tabsystem = 4;
                                }
                            }
                            else
                            {
                                if (ActiveFolderViewIndex != -1)
                                {
                                    SetCursorPos(ImVec2(270.0f, 170.0f));
                                    if (Button("< Go Back", ImVec2(180.0f, 25.0f)))
                                    {
                                        ActiveFolderViewIndex = -1;
                                    }

                                    if (ActiveFolderViewIndex != -1)
                                    {
                                        const auto& currentFolder = folderTabList[ActiveFolderViewIndex];

                                        SetCursorPos(ImVec2(270.0f, 205.0f));
                                        Text("Folder: %s", currentFolder.folderName.c_str());

                                        SetCursorPos(ImVec2(270.0f, 240.0f));
                                        if (BeginTable("##Foliertracktable", 1, ImGuiTableFlags_ScrollY, ImVec2(900.0f * scale_x, 470.0f * scale_y)))
                                        {
                                            TableSetupColumn("Song Title", ImGuiTableColumnFlags_WidthStretch);
                                            TableHeadersRow();

                                            for (size_t k = 0; k < currentFolder.songIndices.size(); k++)
                                            {
                                                int globalSongIdx = currentFolder.songIndices[k];
                                                const auto& track = songlist[globalSongIdx];

                                                TableNextRow(ImGuiTableRowFlags_None, 28.0f);
                                                TableSetColumnIndex(0);

                                                BeginGroup();
                                                float currentX = GetCursorPosX();

                                                SetCursorPosX(currentX + 8.0f * scale_x);
                                                TextDisabled("%d", (int)(k + 1));

                                                SameLine();
                                                SetCursorPosX(currentX + 35.0f * scale_x);

                                                std::string cleanTitle = track.title;
                                                size_t hyphenPos = cleanTitle.find('-');
                                                if (hyphenPos != std::string::npos && hyphenPos < 5)
                                                {
                                                    size_t firstLetterPos = cleanTitle.find_first_not_of("0123456789 ", hyphenPos + 1);
                                                    if (firstLetterPos != std::string::npos)
                                                    {
                                                        cleanTitle = cleanTitle.substr(firstLetterPos);
                                                    }
                                                }

                                                std::string label = cleanTitle + "##fldTrack" + std::to_string(k);
                                                std::string popupId = "##trackPopup" + std::to_string(k);

                                                if (Selectable(label.c_str(), selectedIndex == globalSongIdx, ImGuiSelectableFlags_SpanAllColumns))
                                                {
                                                    selectedIndex = globalSongIdx;
                                                    playerplay(track.path);
                                                    pause = false;
                                                }

                                                if (IsItemClicked(ImGuiMouseButton_Right))
                                                {
                                                    OpenPopup(popupId.c_str());
                                                }

                                                EndGroup();

                                                Dummy(ImVec2(0.0f, 4.0f * scale_y));
                                                Separator();
                                                Dummy(ImVec2(0.0f, 4.0f * scale_y));

                                                PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
                                                if (BeginPopup(popupId.c_str()))
                                                {
                                                    if (!g_Playlists.empty())
                                                    {
                                                        if (BeginMenu("Add to Playlist"))
                                                        {
                                                            for (size_t p = 0; p < g_Playlists.size(); p++)
                                                            {
                                                                if (MenuItem(g_Playlists[p].name.c_str()))
                                                                {
                                                                    auto& vec = g_Playlists[p].songokok;
                                                                    if (std::find(vec.begin(), vec.end(), globalSongIdx) == vec.end())
                                                                    {
                                                                        vec.push_back(globalSongIdx);
                                                                    }
                                                                }
                                                            }
                                                            ImGui::EndMenu(); // had to put imgui in front i was getting this error IMGUI_API void EndMenu(); // only call EndMenu() if BeginMenu() returns true!
                                                        }
                                                    }
                                                    else
                                                    {
                                                        TextDisabled("Create a playlist first");
                                                    }
                                                    EndPopup();
                                                }
                                                PopStyleVar();
                                            }
                                            EndTable();
                                        }
                                    }
                                }
                                
                                else
                                {
                                    SetCursorPos(ImVec2(270.0f, 170.0f));
                                    TextDisabled("All Found Music Directories");

                                    SetCursorPos(ImVec2(270.0f, 210.0f));

                                    ImVec2 windowSize = GetWindowSize();
                                    float childWidth = windowSize.x - (270.0f * scale_x) - 40.0f;
                                    float childHeight = windowSize.y - 300.0f;

                                    if (BeginChild("##FolderList", ImVec2(childWidth, childHeight), false, ImGuiWindowFlags_NoBackground))
                                    {
                                        if (BeginTable("SongTable", 2, ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersInnerH, ImVec2(850.0f * scale_x, childHeight - 20.0f)))
                                        {
                                            TableSetupColumn("Folder name", ImGuiTableColumnFlags_WidthStretch);
                                            TableSetupColumn("Songs", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                                            TableHeadersRow();

                                            for (size_t f = 0; f < folderTabList.size(); f++)
                                            {
                                                TableNextRow(ImGuiTableRowFlags_None, 32.0f);
                                                TableSetColumnIndex(0);

                                                Separator();

                                                std::string label = folderTabList[f].folderName + "##folder" + std::to_string(f);

                                                if (Selectable(label.c_str(), ActiveFolderViewIndex == (int)f, ImGuiSelectableFlags_SpanAllColumns))
                                                {
                                                    ActiveFolderViewIndex = (int)f;
                                                }

                                                TableSetColumnIndex(1);
                                                Text("%d songs", (int)folderTabList[f].songIndices.size());
                                            }
                                            EndTable();
                                        }
                                    }
                                    EndChild();
                                }
                            }
                        }
                    }
                }

                else if (Tabsystem == 2)
                {
                    SetCursorPos(ImVec2(255.0f, 70.0f));
                    PushFont(NULL, 22.0f);
                    Text("Playlists");
                    PopFont();

                    GetWindowDrawList()->AddLine(ImVec2(p.x + 260.0f, p.y + -100.0f), ImVec2(p.x + 1820.0f, p.y + -100.0f), GetColorU32(ImGuiCol_Separator), 1.0f);

                    if (plalistselectedindex != -1)
                    {   
                        ImVec2 windowSize = GetWindowSize();

                        SetCursorPos(ImVec2(270.0f, 120.0f));
                        if (Button("< Go Back", ImVec2(100.0f * scale_x, 22.0f * scale_y)))
                        {
                            plalistselectedindex = -1;
                        }

                        if (plalistselectedindex != -1)
                        {
                            auto& activePlaylist = g_Playlists[plalistselectedindex];

                            SetCursorPos(ImVec2(270.0f , 205.0f * scale_y));
                            if (BeginTable("##idkwhattoputhere", 1, ImGuiTableFlags_ScrollY, ImVec2(780.0f * scale_x, 420.0f * scale_y)))
                            {
                                TableSetupColumn("Song Title",ImGuiTableColumnFlags_WidthStretch); // ImGuiTableFlags_BordersInnerH | 
                                TableHeadersRow();

                                for (size_t s = 0; s < activePlaylist.songokok.size(); s++)
                                {
                                    int songIdx = activePlaylist.songokok[s];
                                    if (songIdx < 0 || songIdx >= (int)songlist.size()) continue;

                                    const auto& track = songlist[songIdx];

                                    TableNextRow(ImGuiTableRowFlags_None, 28.0f * scale_y);
                                    TableSetColumnIndex(0);

                                    Dummy(ImVec2(0.0f, -25.0f * scale_y)); 
                                    Separator(); 
                                    Dummy(ImVec2(0.0f, 4.0f * scale_y));

                                    std::string label = track.title + "##plSong" + std::to_string(s);
                                    if (Selectable(label.c_str(), selectedIndex == songIdx, ImGuiSelectableFlags_SpanAllColumns))
                                    {
                                        selectedIndex = songIdx;
                                        playerplay(track.path);
                                        pause = false;
                                    }
                                }
                                EndTable();
                            }
                        } 
                    }
                    
                    else
                    {
                        if (g_IsMaximized == false)
                            SetCursorPos(ImVec2(960.0f * scale_x, 120.0f));
                        else
                            SetCursorPos(ImVec2(1018.0f * scale_x, 120.0f));

                        SetNextItemWidth(200.0f);
                        PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(30.0f, 8.0f));
                        PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
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

                        if (g_IsMaximized == false)
                            SetCursorPos(ImVec2(970.0f, 128.0f));
                        
                        else
                            SetCursorPos(ImVec2(970.0f * scale_xyzx, 128.0f));
                        Image((ImTextureID)search, ImVec2(16.0f, 16.0f));

                        SetCursorPos(ImVec2(270.0f * scale_x, 170.0f * scale_y));

                        if (g_Playlists.empty())
                        {
                            // Big thanks to GitHub it became clutch it provided me with my old good looking code i am still in a confused state with the scale_x,y etc but it preserved my code so i could just copy paste the old polished part
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
                        
                        else
                        {   
                            // Used LLM for the calculations no way i was doing all this my self too much math but i tweaked it too look nice 
                            float cardWidth = 160.0f * scale_x;
                            float cardHeight = 180.0f * scale_y;
                            float padding = 25.0f * scale_x;

                            float startX = 270.0f * scale_x;
                            float startY = 100.0f * scale_y;
                            
                            float currentX = startX;
                            float currentY = startY;

                            ImVec2 windowSize = GetWindowSize();
                            float availableHeight = windowSize.y - startY - 160.0f * scale_y;

                            if (BeginChild("##asdasdadadlakjldaslk", ImVec2(windowSize.x - startX - 40.0f, availableHeight), false, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysVerticalScrollbar))
                            {
                                float currentX = 0.0f;
                                float currentY = 0.0f;

                                for (size_t i = 0; i < g_Playlists.size(); i++)
                                {
                                    if (strlen(HoldSearch) > 0 && 
                                        strstr(g_Playlists[i].name.c_str(), HoldSearch) == nullptr)
                                        continue;

                                    SetCursorPos(ImVec2(currentX, currentY));

                                    PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
                                    PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.22f, 0.22f, 0.22f, 1.0f));
                                    PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.12f, 0.12f, 0.12f, 1.0f));
                                    PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);

                                    std::string hiddenBtn = "##playlistcard" + std::to_string(i);

                                    if (Button(hiddenBtn.c_str(), ImVec2(cardWidth, cardHeight)))
                                    {
                                        plalistselectedindex = (int)i;
                                    }

                                    PopStyleVar();
                                    PopStyleColor(3);

                                    SetCursorPos(ImVec2(currentX + (cardWidth - 60.0f) * 0.5f, currentY + 25.0f));
                                    Image((ImTextureID)playlist1, ImVec2(60.0f, 60.0f));

                                    ImVec2 textSize = CalcTextSize(g_Playlists[i].name.c_str());
                                    float textX = currentX + (cardWidth - textSize.x) * 0.5f;
                                    SetCursorPos(ImVec2(textX, currentY + 100.0f));
                                    TextUnformatted(g_Playlists[i].name.c_str());

                                    SetCursorPos(ImVec2(currentX + 12.0f, currentY + cardHeight - 35.0f));
                                    PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
                                    Text("%d songs", (int)g_Playlists[i].songokok.size());
                                    PopStyleColor();

                                    currentX += cardWidth + padding;

                                    if (currentX + cardWidth > (windowSize.x - startX - 80.0f))
                                    {
                                        currentX = 0.0f;
                                        currentY += cardHeight + padding + 15.0f;
                                    }
                                }
                            }
                            EndChild();
                            
                        }
                    }
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

                    SetNextItemWidth(200.0f);

                    SetCursorPos(ImVec2(460.0f, 200.0f));

                    PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(15.0f, 8.0f));
                    PushStyleColor(ImGuiCol_Text, ImVec4(0.6, 0.6, 0.6, 1.0f));
                    PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.20f, 0.20f, 0.20f, 1.0f));

                    if (!FoldierPath.empty())
                        strcpy_s(pathBuffer, sizeof(pathBuffer), WStringToString(FoldierPath).c_str());

                    if (InputText("            ", pathBuffer, sizeof(pathBuffer)))
                    {
                        FoldierPath = std::wstring(pathBuffer, pathBuffer + strlen(pathBuffer));
                        SaveMusicPath();
                        songsLoaded = false;
                        scan = false;
                        songlist.clear();
                    }
                    
                    PopStyleVar();
                    PopStyleColor(2);

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
                            clear_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // Here is the issue 
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
            PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);

            SetCursorPos(ImVec2(190.0f, 297.0f));
            if (ImageButton("plus", (ImTextureID)plus, ImVec2(16.0f, 16.0f)))
            {
                // since std::string is fragile and it is easy of name conflict i did this with std because it was getting issues
                Playlist newPlaylist;
                newPlaylist.name = "Playlist " + std::to_string(g_Playlists.size() + 1);
                g_Playlists.push_back(newPlaylist);
            }
            
            PopStyleVar();
            PopStyleColor(3);
            PopFont();

            SetCursorPos(ImVec2(25.0f, 330.0f));

            float newboxw = 200.0f; // here i added offesets for the box that will appear if + is being pressed
            float newboxh = 385.0f;

            if (BeginChild("newbox", ImVec2(newboxw, newboxh), ImGuiChildFlags_None, ImGuiWindowFlags_NoBackground))
            {
                PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
                PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
                PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
                
                PushFont(MyFont);

                for (int i = 0; i < g_Playlists.size(); i++)
                {
                    const bool selecteds = (plalistselectedindex == i);

                    std::string labelID = g_Playlists[i].name + "##item" + std::to_string(i);

                    float selectableWidth = newboxw - 28.0f; 

                    if (Selectable(labelID.c_str(), selecteds, ImGuiSelectableFlags_None, ImVec2(selectableWidth, 0.0f)))
                    {
                        plalistselectedindex = i;
                        Tabsystem = 2; // put it in 2 so it puts us to the playlists tab
                        // LibraryTabSystem = 0;
                    }
                    
                    SameLine(newboxw - 22.0f);

                    std::string closeID = "##close" + std::to_string(i);

                    PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
                    PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
                    PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

                    if (ImageButton(closeID.c_str(), (ImTextureID)closebutton, ImVec2(16.0f, 16.0f)))
                    {
                        g_Playlists.erase(g_Playlists.begin() + i);

                        if (plalistselectedindex >= (int)g_Playlists.size())
                        {
                            plalistselectedindex = (int)g_Playlists.size() - 1;
                        }

                        i--;
                    }
                    
                    PopStyleColor(3);
                }
                
                PopStyleColor(3);
                PopFont();
                EndChild();
            }
        }
        
        EndGroup();
        
            SetCursorPos(ImVec2(230.0f, 38.0f));

            ImVec2 pos = GetCursorScreenPos();
            ImDrawList* draw_list = GetWindowDrawList();
            ImU32 col = GetColorU32(ImGuiCol_Separator);
            draw_list->AddLine(ImVec2(pos.x, pos.y), ImVec2(pos.x, pos.y + GetWindowSize().y - 38.0f), col, 1.0f);

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

    asdasdcleanup();

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
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

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
    // i tried without it and i found on a forum that i need this without this it wouldnt work at all
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
        case WM_KEYDOWN:
        if (wParam == VK_SPACE) // You dont want to know how much time it took me to find the docs for this here is link i may use it in the future https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
        {
            if (selectedIndex >= 0 && selectedIndex < (int)songlist.size())
            {
                pause = !pause;

                if (pause)
                    playerpause();
                else
                {
                    if (pause == false)
                        playerresume();
                }
            }
            
            else if (!songlist.empty())
            {
                selectedIndex = 0;
                if (playerplay(songlist[0].path))
                    pause = false;
            }
            return 0;
        }
        
        if (wParam == VK_MEDIA_NEXT_TRACK)
        {
            if (!songlist.empty())
            {
                if (selectedIndex < (int)songlist.size())
                {
                    selectedIndex++;
                }

                else
                {
                    selectedIndex = 0;
                }

                playerplay(songlist[selectedIndex].path);
                pause = false;
            }
        }
        
        if (wParam == VK_MEDIA_PREV_TRACK)
        {
            if (!songlist.empty())
            {
                if (selectedIndex < (int)songlist.size())
                {
                    selectedIndex--;
                }

                else
                {
                    selectedIndex = 0;
                }

                playerplay(songlist[selectedIndex].path);
                pause = false;
            }
        }
        break;
    
        
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
            return 0;

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

// if anyone reached down here vs code should be lagging 
