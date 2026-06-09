#pragma once
#include <string>
#include <fstream>

bool WinInetInit();
bool DownloadImg(const std::string& url, const std::string& folderPath);