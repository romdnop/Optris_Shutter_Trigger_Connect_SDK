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
		/*SetConsoleCursorPosition(hStdout, CursorPosition);
		printf("Frame counter HW/SW: %d/%d\n", pMetadata->CounterHW, pMetadata->Counter);
        printf("PIF");
        for (int i = 0; i < pMetadata->PIFnDI; i++)
            printf("  DI%d:%d", i + 1, (pMetadata->PIFDI >> i) & 1);
        for (int i = 0; i < pMetadata->PIFnAI; i++)
            printf("  AI%d:%d", i + 1, pMetadata->PIFAI[i]);
        printf("\n");
		printf("Target-Temp: %3.1f\370C\n", (float)GetTempTarget(0));
		printf("Flag: |%s|      <-- Hit SPACE to renew flag\n", &s[min(int(pMetadata->FlagState),4)*14]);*/
		std::string timestamp = GetCurrentTimestamp();
		printf("Filename: %s\n", timestamp.c_str());
		//FileSnapshot(0);
		//Sleep(400);
		framesReceivedCnt++;	
	}
	return 0;
}

HRESULT WINAPI OnFileCommandReady(wchar_t* savedFilePath)
{
	//GetPathOfStoredFile(0, savedFilePath, 512);
	std::string folderPath = "C:\\Users\\User\\Documents\\Imager Data"; // Replace with the actual folder path
	//std::string destinationFolder = "C:\\Users\\User\\Documents\\Imager Data\\exported"; // Replace with destination path
	//std::string destinationFolder = "C:\\DestinationFolder"; // Replace with destination folder path


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

	/*
	while (!GetAsyncKeyState(VK_ESCAPE) && !Stopped) // loop until ESC is pressed or stopped by server
	{
		ImagerIPCProcessMessages(0);
		if (GetAsyncKeyState(VK_SPACE)) // renew flag if SPACE is pressed
			RenewFlag(0);
		if (framesReceivedCnt >= 100)
		{
			return 0;
		}
		if (snaphotSaved != true)
		{
			Sleep(SNAPSHOT_READY_CHECK_STEP);
			if (snapshotCheckTimeOvf >= MAX_SNAPHOT_DELAY)
			{
				return 0;
			}
			snapshotCheckTimeOvf += SNAPSHOT_READY_CHECK_STEP;
		}
	}
	*/
	/*
	if (RunImagerIPC(0) < 0)
	{
		ReleaseImagerIPC(0);
		printf("\nRun failed! Press Enter to exit...");
		getchar();
		return -1;
	}

    hStdout = GetStdHandle(STD_OUTPUT_HANDLE); 
	CONSOLE_SCREEN_BUFFER_INFO CSBI;
	GetConsoleScreenBufferInfo(hStdout, &CSBI); // memorize last cursor pos to repeat output 
	CursorPosition = CSBI.dwCursorPosition;   

	while(!GetAsyncKeyState(VK_ESCAPE) && !Stopped) // loop until ESC is pressed or stopped by server
	{
		ImagerIPCProcessMessages(0);
		if(GetAsyncKeyState(VK_SPACE)) // renew flag if SPACE is pressed
 			RenewFlag(0);
	}
	printf("\n                    "); // clear last line
	if(Stopped)
	{
		printf("\nIPC stopped by server! Press Enter to exit...");
		getchar();
	}

	ReleaseImagerIPC(0);
	return 0;
	*/
}
