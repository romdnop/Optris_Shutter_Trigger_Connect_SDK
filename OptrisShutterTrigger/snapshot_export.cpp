#include "snapshot_export.h"

std::string outputFileName;

std::string GetCurrentTimestamp()
{
	SYSTEMTIME st;
	GetLocalTime(&st); // Retrieves the current local date and time.

	// Format the timestamp as yyyyMMdd_hhHHss
	std::ostringstream timestamp;
	timestamp << std::setfill('0') << std::setw(4) << st.wYear
		<< std::setw(2) << st.wMonth
		<< std::setw(2) << st.wDay
		<< "_"
		<< std::setw(2) << st.wHour
		<< std::setw(2) << st.wMinute
		<< std::setw(2) << st.wSecond;

	return timestamp.str();
}

bool FindLatestModifiedFile(const std::string& folderPath, std::string& latestFile) {
    WIN32_FIND_DATAA findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;

    // Append wildcard to search all files in the directory
    std::string searchPath = folderPath + "\\*";

    hFind = FindFirstFileA(searchPath.c_str(), &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        std::cerr << "Error: Unable to access folder: " << folderPath << std::endl;
        return false;
    }

    FILETIME latestTime = { 0, 0 }; // Initialize the latest file time
    do {
        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // Skip directories
            continue;
        }

        if (CompareFileTime(&findFileData.ftLastWriteTime, &latestTime) > 0) {
            // Update latest file and time
            latestTime = findFileData.ftLastWriteTime;
            latestFile = folderPath + "\\" + findFileData.cFileName;
        }
    } while (FindNextFileA(hFind, &findFileData) != 0);

    FindClose(hFind);
    return !latestFile.empty();
}

bool CopyFileToPath(const std::string& sourcePath, const std::string& destinationFolder) {
    // Extract the file name from the source path
    // Extract the file name from the source path
    size_t lastSlash = sourcePath.find_last_of("\\/");
    std::string fileName;
    if (outputFileName.empty())
    {
        fileName = (lastSlash != std::string::npos) ? sourcePath.substr(lastSlash + 1) : sourcePath;
    }
    else
    {
        fileName = outputFileName;
    }
    //std::string 

    // Construct the full destination path
    std::string destinationPath = destinationFolder + "\\" + fileName;

    if (!CopyFileA(sourcePath.c_str(), destinationPath.c_str(), FALSE)) {
        std::cerr << "Error: Unable to copy file from " << sourcePath
            << " to " << destinationPath << ". Error code: " << GetLastError() << std::endl;
        return false;
    }
    return true;
}

std::string WCharToString(const wchar_t* wcharStr) {
    if (!wcharStr) {
        return "";
    }

    int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wcharStr, -1, nullptr, 0, nullptr, nullptr);
    std::string result(bufferSize, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wcharStr, -1, &result[0], bufferSize, nullptr, nullptr);

    // Remove the null terminator included by WideCharToMultiByte
    result.pop_back();

    return result;
}
