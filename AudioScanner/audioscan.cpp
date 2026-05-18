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

string getname (const string& path)
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
    if (FoldierPath.empty())
    {
        // clear song list
        songlist.clear();
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

void ldsngsifneeded() 
{
    if (songsLoaded || FoldierPath.empty()) return;
    
    scanfiles();
    songsLoaded = true;
}
