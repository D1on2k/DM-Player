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

#include "audioscan.h"

using namespace std;
using namespace filesystem;

const vector<string> formats = {".mp3", ".wav", ".flac", ".m4a", ".ogg", ".aac"};

// use algorithm so it can detect lower and upper case
string uplow(const string& str)
{
    string result = str;
    
    transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return tolower(c);});
    
    return result;
}

string getname(const string& path)
{
    size_t lastSlash = path.find_last_of("/\\");
    string filename = (lastSlash != string::npos) ? path.substr(lastSlash + 1) : path;
    size_t lastDot = filename.find_last_of('.');
    
    if (lastDot != string::npos) 
    {
        filename = filename.substr(0, lastDot);
    }
    return filename;
}

void scanfiles()
{
    songlist.clear();

    if (FoldierPath.empty()) 
    {
        return;
    }

    try
    {
        path p(FoldierPath);

        if (exists(p) && is_directory(p))
        {
            for (const auto& entry : directory_iterator(p)) 
            {
                if (entry.is_regular_file()) 
                {
                    string ext = entry.path().extension().string();
                    
                    for (const auto& audioExt : formats) 
                    {
                        if (uplow(ext) == uplow(audioExt)) 
                        {
                            // make song entry
                            SongDisplay song;
                            song.path = entry.path().string();
                            song.title = getname(entry.path().string());
                            song.artist = "Unknown Artist";
                            song.album = "Unknown Album";
                            
                            songlist.push_back(song);
                            break;
                        }
                    }
                }
            }
        }
    }
    catch (...) 
    {

    }
}

// stack overflow shoutout i found what i needed and i had to do minimal changes to make it work
void scanfoldiers()
{
    folderTabList.clear();
    songlist.clear();

    if (FoldierPath.empty())
    {
        return;
    }

    try 
    {
        path baseDir(FoldierPath);
        if (!exists(baseDir) || !is_directory(baseDir))
        {
            return;
        }
        
        for (const auto& entry : recursive_directory_iterator(baseDir))
        {
            if (entry.is_regular_file())
            {
                string ext = entry.path().extension().string();
                bool isAudio = false;

                for (const auto& audioExt : formats)
                {
                    if (uplow(ext) == uplow(audioExt))
                    {
                        isAudio = true;
                        break;
                    }
                }

                if (isAudio)
                {
                    SongDisplay song;
                    song.path = entry.path().string();
                    song.title = getname(entry.path().string());
                    song.artist = "Unknown Artist";
                    song.album = "Unknown Album";

                    songlist.push_back(song);
                    int newlyAddedSongIndex = (int)songlist.size() - 1;

                    path parentFolder = entry.path().parent_path();
                    string parentFolderPathStr = parentFolder.string();

                    bool folderExists = false;
                    for (size_t f = 0; f < folderTabList.size(); f++)
                    {
                        if (folderTabList[f].fullFolderPath == parentFolderPathStr)
                        {
                            folderTabList[f].songIndices.push_back(newlyAddedSongIndex);
                            folderExists = true;
                            break;
                        }
                    }

                    if (!folderExists)
                    {
                        FolderDisplay newFolder;
                        newFolder.folderName = parentFolder.filename().string();
                        newFolder.fullFolderPath = parentFolderPathStr;
                        newFolder.songIndices.push_back(newlyAddedSongIndex);
                        newFolder.folderImg = nullptr;

                        DownImage(parentFolderPathStr);

                        folderTabList.push_back(newFolder);
                    }
                }
            }
        }
    }
    catch (...)
    {

    }
}

void ldsngsifneeded() 
{
    if (songsLoaded || FoldierPath.empty()) return;
    
    scanfiles();
    scanfoldiers(); // almost forgot it without this we would scna nothing

    songsLoaded = true;
}
