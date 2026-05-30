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

#include "select_path.h"

using namespace std;

std::wstring FoldierPath = L"";

// Helper to convert wide string to normal string (we need this sometimes)
string WStringToString(const wstring& wstr)
{
    if (wstr.empty()) return "";
    
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    string result(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], size, NULL, NULL);
    result.resize(size - 1);
    return result;
}


bool SelectMusicFolder()
{
    // Setup the folder dialog
    BROWSEINFOW bi = { 0 };
    bi.lpszTitle = L"Select Folder";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE; // Nice looking dialog

    // Open the folder picker
    LPITEMIDLIST pidl = SHBrowseForFolderW(&bi);
    
    if (pidl != nullptr)
    {
        wchar_t path[MAX_PATH] = { 0 };
        
        if (SHGetPathFromIDListW(pidl, path))
        {
            FoldierPath = path; // Save the selected path
            SaveMusicPath(); // Save it to file so it remembers next time
            CoTaskMemFree(pidl);
            return true;
        }
        CoTaskMemFree(pidl);
    }
    
    return false;
}

void SaveMusicPath()
{
    std::wofstream file("music_path.txt"); 
    
    if (file.is_open())
    {
        file << FoldierPath;
        file.close();
    }
}

void LoadSavedMusicPath()
{
    std::wifstream file("music_path.txt"); 
    
    if (file.is_open())
    {
        std::getline(file, FoldierPath); 
        file.close();
    }
}
// used llm here to convert i couldnt find a way to get non english characters working
std::string WStringToUTF8(const std::wstring& wstr)
{
    if (wstr.empty()) return "";
    
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    if (size <= 0) return "";
    
    std::string result(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], size, NULL, NULL);
    
    // Resize to trim the null-terminator safely
    result.resize(size - 1);
    return result;
}
