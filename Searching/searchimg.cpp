#include <string>
#include <filesystem>
#include <vector>
#include <algorithm>
#include "searching.h"

using namespace std;
using namespace filesystem;

string takepath = "";
string searchfortitle = "";
string searchpathformusic = "";

void findimages() 
{   
    // Check if a image exists 
    vector<string> extensions = { ".png", ".jpg", ".jpeg", ".webp" };

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