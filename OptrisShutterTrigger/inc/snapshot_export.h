#pragma once

#ifndef _EXPORT_H
#define _EXPORT_H

#include <windows.h>
#include <iostream>
#include <iomanip>
#include <sstream>

extern std::string outputFileName;

std::string GetCurrentTimestamp();
bool FindLatestModifiedFile(const std::string& folderPath, std::string& latestFile);
bool CopyFileToPath(const std::string& sourcePath, const std::string& destinationPath);
std::string WCharToString(const wchar_t* wcharStr);

#endif // !1

