#include "select_path.h"

using namespace std;

string FoldierPath = "";

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
    
    BROWSEINFOA bi = { 0 };
    bi.lpszTitle = "Select Folder";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;   // Nice looking dialog

    // Open the folder picker
    LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
    
    if (pidl != nullptr)
    {
        char path[MAX_PATH] = { 0 };
        
        if (SHGetPathFromIDListA(pidl, path))
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
    ofstream file("music_path.txt");
    
    if (file.is_open())
    {
        file << FoldierPath;
        file.close();
    }
}

void LoadSavedMusicPath()
{
    ifstream file("music_path.txt");
    
    if (file.is_open())
    {
        getline(file, FoldierPath);
        file.close();
    }
}
