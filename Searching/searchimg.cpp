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

#include <iostream>
#include <string>
#include <filesystem>
#include <vector>
#include <algorithm>
#include "searching.h"
#include "PathTab4/select_path.h"

using namespace std;
using namespace filesystem;

string takepath = "";
string searchfortitle = "";
string searchpathformusic = "";
string foldiername = "";

// Used Llm for this i could not find another way
pair<string, string> FolderName(const string& folderName)
{
    size_t pos = folderName.find_last_of("-");
   
    if (pos != string::npos)
    {
        string artist = folderName.substr(0, pos);
        string album = folderName.substr(pos + 1);

        if (!album.empty() && album[0] == ' ')
            album = album.substr(1);
            
        return make_pair(artist, album);
    }
    

    return make_pair(folderName, "Unknown Album");
}

void findimages() 
{   
    path p(FoldierPath);
    searchpathformusic = p.string();
    
    foldiername = p.filename().string(); // get the name of the foldier

    pair<string, string> parsedName = FolderName(foldiername);

    searchpathformusic = WStringToString(FoldierPath);

    vector<string> extensions = { ".png", ".jpg", ".jpeg", ".webp" };

    // Check if a image exists 
    try 
    {
        if (exists(searchpathformusic) && is_directory(searchpathformusic)) 
        {
            for (const auto& entry : directory_iterator(searchpathformusic)) 
            {
                if (entry.is_regular_file()) 
                {
                    string ext = entry.path().extension().string();
                    
                    // Make it lower case so it doesnt skip jpg
                    for (auto &c : ext) c = tolower(c);

                    for (const string& v : extensions) 
                    {
                        if (ext == v) 
                        {
                            takepath = entry.path().string();
                            searchfortitle = entry.path().stem().string();
                            return; 
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

bool idk12123(const std::string& folderName, std::string& artist, std::string& album) 
{
    std::pair<std::string, std::string> Name = FolderName(folderName);
    
    artist = Name.first;
    album = Name.second;
    
    return !artist.empty();
}

// might as well put this here 
std::string FormatTime(float seconds)
{
    if (seconds < 0) 
    {
        seconds = 0;
    }
    
    int minutes = (int)seconds / 60;
    int secs = (int)seconds % 60;
    
    char buf[16];
    
    sprintf_s(buf, "%02d:%02d", minutes, secs);
    
    return std::string(buf);
}

std::vector<std::string> searchinfoldier(const std::string& folderPath)
{
    std::vector<std::string> all_found_images;
    
    if (folderPath.empty()) return all_found_images;
    try 
    {
        std::filesystem::path targetPath(folderPath);

        if (std::filesystem::exists(targetPath) && std::filesystem::is_directory(targetPath)) 
        {
            std::vector<std::string> extensions = { ".png", ".jpg", ".jpeg", ".webp" };

            for (const auto& entry : std::filesystem::directory_iterator(targetPath)) 
            {
                if (entry.is_regular_file()) 
                {
                    std::string ext = entry.path().extension().string();
                    for (auto &c : ext) c = std::tolower(c);

                    for (const std::string& v : extensions) 
                    {
                        if (ext == v) 
                        {
                            all_found_images.push_back(entry.path().string());
                            break;
                        }
                    }
                }
            }
        }
    } 
    catch (...) {}
    
    return all_found_images;
}
