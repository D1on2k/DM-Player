// Select_path.h

#pragma once

#include <windows.h>
#include <shlobj.h>
#include <fstream>
#include <string>

extern std::string FoldierPath;

bool SelectMusicFolder();
void LoadSavedMusicPath();
void SaveMusicPath();
