// searching.h

#pragma once

#define _HAS_STD_BYTE 0

#include <string>
#include "PathTab4/select_path.h"
#include <utility>

extern std::string takepath;
extern std::string searchfortitle;
extern std::string searchpathformusic;
extern std::string foldiername;

void findimages();
bool idk12123(const std::string& folderName, std::string& artist, std::string& album);
