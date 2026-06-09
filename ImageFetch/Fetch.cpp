#include "Fetch.h"
#include <windows.h>
#include <wininet.h>
#include <fstream>
#include <iostream>
#include <filesystem>

#pragma comment(lib, "wininet.lib")

HINTERNET hInternet = nullptr;

bool WinInetInit()
{
    if (hInternet) return true;
    hInternet = InternetOpenA("MusicCoverDownloader", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    return hInternet != nullptr;
}

bool DownloadImg(const std::string& url, const std::string& folderPath)
{
    if (!WinInetInit()) return false;

    HINTERNET hConnect = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0,
        INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);

    if (!hConnect) return false;

    std::string filename = "Cover.jpg"; // If anybody wants to save space or anything they could change .jpg to png webp(great for storage) and any other image format
    size_t lastSlash = url.find_last_of('/');
    if (lastSlash != std::string::npos)
    {
        std::string name = url.substr(lastSlash + 1);
        if (!name.empty() && name.find('.') != std::string::npos)
            filename = name;
    }

    std::filesystem::path fullPath = std::filesystem::path(folderPath) / filename;

    std::ofstream outputFile(fullPath, std::ios::binary);
    if (!outputFile)
    {
        InternetCloseHandle(hConnect);
        return false;
    }

    char buffer[8192];
    DWORD bytesRead = 0;
    while (InternetReadFile(hConnect, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0)
    {
        outputFile.write(buffer, bytesRead);
    }

    outputFile.close();
    InternetCloseHandle(hConnect);

    std::cout << "Downloaded: " << fullPath << std::endl;
    return true;
}