#include "stdafx.h"
#include <windows.h>
#include "ImagerIPC2.h"
#include "snapshot_export.h"


HANDLE hStdout;
COORD CursorPosition = {0, 0};
bool frameInitialized = false, Connected = false, Stopped = false;
short FrameWidth, FrameHeight, FrameDepth;
int FrameSize;
const char s[] = {"open         \0       closed\0 <-opening   \0   closing-> \0             \0"};
int framesReceivedCnt = 0;
extern bool snaphotSaved = false;
const int MAX_SNAPHOT_DELAY = 5000;
const int SNAPSHOT_READY_CHECK_STEP = 10;
bool snaphotCmdSent = false;
bool takeSnapshotNow = false;

std::string destinationFolder;

HRESULT WINAPI OnServerStopped(int reason) { Stopped = true; return 0; }
HRESULT WINAPI OnInitCompleted(void) { Connected = true; takeSnapshotNow = true; return 0; }
HRESULT WINAPI OnFrameInit(int frameWidth, int frameHeight, int frameDepth) 
{
	FrameWidth = frameWidth;
	FrameHeight = frameHeight;
	FrameDepth = frameDepth;
	FrameSize = FrameWidth * FrameHeight;
	frameInitialized = true;
	//printf("------------------------------------------------------\nWidth,Height: (%d,%d)\n\n\n\n\n", FrameWidth, FrameHeight);
	//printf("------------------------------------------------------\nHit ESC to exit...\n");
	CursorPosition.Y += 2;
	return 0;
}

HRESULT WINAPI OnNewFrame(void* pBuffer, FrameMetadata2 *pMetadata) 
{
	if(frameInitialized && Connected)
	{
		std::string timestamp = GetCurrentTimestamp();
		printf("Filename: %s\n", timestamp.c_str());
		framesReceivedCnt++;	
	}
	return 0;
}

HRESULT WINAPI OnFileCommandReady(wchar_t* savedFilePath)
{
	std::string folderPath = "C:\\Users\\User\\Documents\\Imager Data"; // Replace with the actual folder path

	if (CopyFileToPath(WCharToString(savedFilePath), destinationFolder)) {
		std::cout << "File successfully copied to: " << destinationFolder << std::endl;
	}
	else {
		std::cerr << "No files found in the folder or an error occurred." << std::endl;
	}
	snaphotSaved = true;
	return 0;
}


// Function to display help message
void DisplayHelp() {
	std::cout << "Usage: OptrisShutterTrigger.exe [options]\n\n"
		<< "Options:\n"
		<< "  -e <path>      Specify the destination folder for the file copy.\n"
		<< "  -t <milliseconds> Add a delay (in milliseconds) before copying the file.\n"
		<< "  -n <file name> Set output file name\n"
		<< "  --help         Display this help message.\n\n"
		<< "Example:\n"
		<< "  OptrisShutterTrigger.exe -e \"C:\\Path With Spaces\\DestinationFolder\" -t 5000\n"
		<< "    Copies the latest modified file to the specified destination with a 5-second delay.\n";
}


int main(int argc, char* argv[]) 
{
	
	DWORD snapshotDelay = 0;
	// Parse command-line arguments
	for (int i = 1; i < argc; ++i) {
		if (std::string(argv[i]) == "-e" && i + 1 < argc) {
			destinationFolder = argv[i + 1];
			++i; // Skip the next argument as it's already processed
		}
		else if (std::string(argv[i]) == "-t" && i + 1 < argc) {
			snapshotDelay = std::strtoul(argv[i + 1], nullptr, 10); // Convert argument to DWORD
			++i; // Skip the next argument as it's already processed
		}
		else if (std::string(argv[i]) == "-n" && i + 1 < argc) {
			outputFileName = argv[i + 1];
			++i; // Skip the next argument as it's already processed
		}
		else if (std::string(argv[i]) == "--help") {
			DisplayHelp();
			return 0; // Exit after displaying help
		}
		else {
			std::cerr << "Unknown argument: " << argv[i] << "\nUse --help to display usage information." << std::endl;
			return 1;
		}
	}

	if (argc == 1) {
		std::cout << "No arguments provided.\n";
		DisplayHelp();
		return 0; // Exit after displaying help
	}

	// Validate the destination folder
	if (destinationFolder.empty()) {
		std::cerr << "Error: No export path provided. Use -e <path> to specify the export path." << std::endl;
		return 1;
	}

	if (InitImagerIPC(0) < 0) 
	{
		printf("\nInit failed! Press Enter to exit...");
		printf("\nCheck if PIX connect is running in 'Connect SDK (IPC) mode!'");
		printf("\nTools - Configuration - External Communication - Connect SDK");
		getchar();
		return -1;
	}
	SetCallback_OnServerStopped(0, &OnServerStopped); 
	SetCallback_OnFrameInit(0, &OnFrameInit);
	SetCallback_OnNewFrameEx2(0, &OnNewFrame);
	SetCallback_OnInitCompleted(0, &OnInitCompleted);
	SetCallback_OnFileCommandReady(0, &OnFileCommandReady);

	SetIPCMode(0, 1); //0 - Colour, 1 - Temp, 2 - ADU


	if (RunImagerIPC(0) < 0)
	{
		ReleaseImagerIPC(0);
		printf("\nRun failed! Press Enter to exit...");
		getchar();
		return -1;
	}

	int snapshotCheckTimeOvf = 0;

	snaphotSaved = false;
	
	while (!snaphotSaved && !Stopped)
	{
		ImagerIPCProcessMessages(0);
		if (takeSnapshotNow == true && snaphotCmdSent != true)

		{
			if (snapshotDelay > 0)
			{
				Sleep(snapshotDelay);
			}
			FileSnapshot(0);
			printf("\nSnaphot command sent!\n");
			printf("\nSnapshot delay: %d ms\n", snapshotDelay);
			snaphotCmdSent = true;
			continue;
		}
		if (snaphotSaved)
		{
			break;
		}
		Sleep(SNAPSHOT_READY_CHECK_STEP);
		if (snapshotCheckTimeOvf >= MAX_SNAPHOT_DELAY)
		{
			break;
		}
		snapshotCheckTimeOvf += SNAPSHOT_READY_CHECK_STEP;
	}
	ReleaseImagerIPC(0);
	return 0;
}
