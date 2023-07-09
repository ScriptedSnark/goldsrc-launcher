#include <cstdio>
#include <cstdlib>
#include <cstring>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>

#include <io.h>

#include "engine_launcher_api.h"
#include "FileSystem.h"
#include "ICommandLine.h"
#include "interface.h"
#include "IRegistry.h"

#include "MinHook.h"
#include "goldsrc_hook.h"

#define HARDWARE_ENGINE "hw.dll"
#define SOFTWARE_ENGINE "sw.dll"

char com_gamedir[MAX_PATH];
IFileSystem* g_pFileSystem = nullptr;

//-----------------------------------------------------------------------------
// Getting executable name
//-----------------------------------------------------------------------------
bool Sys_GetExecutableName(char* out, int len)
{
	if (::GetModuleFileName((HINSTANCE)GetModuleHandle(NULL), out, len))
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Handler routine for allocated console in text mode
//-----------------------------------------------------------------------------
BOOL WINAPI MyHandlerRoutine(DWORD dwCtrlType)
{
	TerminateProcess(GetCurrentProcess(), 2);
	return TRUE;
}

//-----------------------------------------------------------------------------
// Initialize text mode (allocate console and print any output)
//-----------------------------------------------------------------------------
void InitTextMode()
{
	AllocConsole();

	SetConsoleCtrlHandler(MyHandlerRoutine, TRUE);

	freopen("CONIN$", "rb", stdin);	  // reopen stdin handle as console window input
	freopen("CONOUT$", "wb", stdout); // reopen stout handle as console window output
	freopen("CONOUT$", "wb", stderr); // reopen stderr handle as console window output
}

//-----------------------------------------------------------------------------
// Loading filesystem_stdio.dll (GoldSrc file system)
//-----------------------------------------------------------------------------
CSysModule* LoadFilesystemModule(const char* exename)
{
	CSysModule* pModule = Sys_LoadModule("filesystem_stdio.dll");

	if (pModule)
		return pModule;

	if (strchr(exename, ';'))
	{
		MessageBox(NULL, "Game cannot be run from directories containing the semicolon char", "Fatal Error", MB_ICONERROR);
		return nullptr;
	}

	struct _finddata_t find_data;
	intptr_t result = _findfirst("filesystem_stdio.dll", &find_data);

	if (result == -1)
		MessageBox(NULL, "Could not find FileSystem_stdio.dll to load.", "Fatal Error", MB_ICONERROR);
	else
	{
		MessageBox(NULL, "Could not load FileSystem_stdio.dll.", "Fatal Error", MB_ICONERROR);
		_findclose(result);
	}

	return pModule;
}

//-----------------------------------------------------------------------------
// Get base directory (directory where hl.exe is running from)
//-----------------------------------------------------------------------------
char* UTIL_GetBaseDir(void)
{
	static char basedir[MAX_PATH];
	char szFilename[MAX_PATH], *pBuffer;

	if (GetModuleFileName(NULL, szFilename, sizeof(szFilename)))
	{
		GetLongPathName(szFilename, basedir, sizeof(basedir));

		pBuffer = strrchr(basedir, '\\');

		if (*pBuffer)
			*(pBuffer + 1) = '\0';

		int j = strlen(basedir);

		if (j > 0)
		{
			if ((basedir[j - 1] == '\\') || (basedir[j - 1] == '/'))
				basedir[j - 1] = 0;
		}
	}

	return basedir;
}

//-----------------------------------------------------------------------------
// Set engine DLL for loading
//-----------------------------------------------------------------------------
void SetEngineDLL(const char** ppEngineDLL)
{
	const char* pEngineDLLSetting = registry->ReadString("EngineDLL", HARDWARE_ENGINE);

	*ppEngineDLL = HARDWARE_ENGINE;

	if (!stricmp(pEngineDLLSetting, HARDWARE_ENGINE))
	{
		*ppEngineDLL = HARDWARE_ENGINE;
	}
	else if (!stricmp(pEngineDLLSetting, SOFTWARE_ENGINE))
	{
		*ppEngineDLL = SOFTWARE_ENGINE;
	}

	if (cmdline->CheckParm("-soft", nullptr) || cmdline->CheckParm("-software", nullptr))
	{
		*ppEngineDLL = SOFTWARE_ENGINE;
	}
	else if (cmdline->CheckParm("-gl", nullptr) || cmdline->CheckParm("-d3d", nullptr))
	{
		*ppEngineDLL = HARDWARE_ENGINE;
	}

	registry->WriteString("EngineDLL", *ppEngineDLL);
}

//-----------------------------------------------------------------------------
// When video mode is failed, execute this code
//-----------------------------------------------------------------------------
bool OnVideoModeFailed(void)
{
	registry->WriteInt("ScreenBPP", 16);
	registry->WriteInt("ScreenHeight", 640);
	registry->WriteInt("ScreenWidth", 480);

	registry->WriteString("EngineDLL", HARDWARE_ENGINE);

	return MessageBox(NULL, "The specified video mode is not supported.", "Video mode change failure", MB_OKCANCEL | MB_ICONERROR | MB_ICONQUESTION) == IDOK;
}

//-----------------------------------------------------------------------------
// The real entry point for hl.exe
//-----------------------------------------------------------------------------
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	static char szNewCommandParams[2048];
	char szFilename[256];

#ifndef DEBUG
	HANDLE hMutex = CreateMutex(nullptr, FALSE, "ValveHalfLifeLauncherMutex");

	if (NULL != hMutex)
		GetLastError();

	DWORD dwMutexResult = ::WaitForSingleObject(hMutex, 0);

	if (dwMutexResult != WAIT_OBJECT_0 && dwMutexResult != WAIT_ABANDONED)
	{
		MessageBoxA(NULL, "Could not launch game.\nOnly one instance of this game can be run at a time.", "Fatal Error", MB_OK | MB_ICONERROR);
		return EXIT_SUCCESS;
	}
#endif

	MH_Initialize();

	WSADATA WSAData;
	::WSAStartup(MAKEWORD(2, 0), &WSAData);

	registry->Init(); 

	cmdline->CreateCmdLine(GetCommandLineA());

	Sys_GetExecutableName(szFilename, sizeof(szFilename));

	char* pszLastSlash = strrchr(szFilename, '\\') + 1;

	if (stricmp("hl.exe", pszLastSlash) && !cmdline->CheckParm("-game", nullptr))
	{
		pszLastSlash[strlen(pszLastSlash) - 4] = 0;
		cmdline->SetParm("-game", pszLastSlash);
	}

	const char* pszGame = cmdline->CheckParm("-game", nullptr);

	if (pszGame)
		strncpy(com_gamedir, pszGame, sizeof(com_gamedir));
	else
		strcpy(com_gamedir, "valve");

	if (cmdline->CheckParm("-textmode", nullptr))
		InitTextMode();

	// Make low priority?
	if (cmdline->CheckParm("-low", nullptr))
		SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);
	else if (cmdline->CheckParm("-high", nullptr))
		SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

#ifndef DEBUG
	_unlink("mssv29.asi");
	_unlink("mssv12.asi");
	_unlink("mp3dec.asi");
	_unlink("opengl32.dll");
#endif

	if (registry->ReadInt("CrashInitializingVideoMode", 0))
	{
		registry->WriteInt("CrashInitializingVideoMode", 0);

		const char* pszEngineDLL = registry->ReadString("EngineDLL", HARDWARE_ENGINE);

		if (!stricmp(pszEngineDLL, HARDWARE_ENGINE))
		{
			const char* pszMessage;

			if (registry->ReadInt("EngineD3D", 0))
			{
				registry->WriteInt("EngineD3D", 0);

				pszMessage =
					"The game has detected that the previous attempt to start in D3D video mode failed.\n"
					"The game will now run attempt to run in openGL mode.";
			}
			else
			{
				registry->WriteString("EngineDLL", HARDWARE_ENGINE);

				pszMessage =
					"The game has detected that the previous attempt to start in openGL video mode failed.\n"
					"The game will now run in software mode.";
			}

			if (MessageBox(NULL, pszMessage, "Video mode change failure", MB_OKCANCEL | MB_ICONERROR | MB_ICONQUESTION) != IDOK)
				return EXIT_SUCCESS;

			registry->WriteInt("ScreenBPP", 16);
			registry->WriteInt("ScreenHeight", 640);
			registry->WriteInt("ScreenWidth", 480);
		}
	}

	int runResult;
	bool bRestartEngine = false;

	do
	{
		CSysModule* hModule = LoadFilesystemModule(szFilename);

		if (!hModule)
			break;

		CreateInterfaceFn factoryFn = Sys_GetFactory(hModule);

		g_pFileSystem = (IFileSystem*)factoryFn(FILESYSTEM_INTERFACE_VERSION, nullptr);
		g_pFileSystem->Mount();
		g_pFileSystem->AddSearchPath(UTIL_GetBaseDir(), "ROOT");

		bRestartEngine = false;
		runResult = 0;
		szNewCommandParams[0] = 0;

		const char* pszLibFileName;
		SetEngineDLL(&pszLibFileName);

		CSysModule* hLibModule = Sys_LoadModule(pszLibFileName);

		HookSDL2();
		HookEngine(); // it must be here!!!
		MH_EnableHook(MH_ALL_HOOKS);

		if (hLibModule)
		{
			CreateInterfaceFn factoryFn = Sys_GetFactory(hLibModule);

			if (factoryFn)
			{
				IEngineAPI* pEngine = static_cast<IEngineAPI*>(factoryFn(ENGINE_LAUNCHER_INTERFACE_VERSION, nullptr));

				if (pEngine)
					runResult = pEngine->Run(hInstance,	UTIL_GetBaseDir(), cmdline->GetCmdLine(), szNewCommandParams, Sys_GetFactoryThis(), Sys_GetFactory(hModule));
			}

			Sys_UnloadModule(hLibModule);
		}
		else
		{
			char Text[512];

			snprintf(Text, sizeof(Text), "Could not load %s.", pszLibFileName);
			MessageBoxA(NULL, Text, "Fatal Error", MB_ICONERROR);
		}

		switch (runResult)
		{
		case 0:
			bRestartEngine = false;
			break;
		case 1:
			bRestartEngine = true;
			break;
		case 2:
			bRestartEngine = OnVideoModeFailed();
			break;
		}

		cmdline->RemoveParm("-sw");
		cmdline->RemoveParm("-startwindowed");
		cmdline->RemoveParm("-windowed");
		cmdline->RemoveParm("-window");
		cmdline->RemoveParm("-full");
		cmdline->RemoveParm("-fullscreen");
		cmdline->RemoveParm("-soft");
		cmdline->RemoveParm("-software");
		cmdline->RemoveParm("-gl");
		cmdline->RemoveParm("-d3d");
		cmdline->RemoveParm("-w");
		cmdline->RemoveParm("-width");
		cmdline->RemoveParm("-h");
		cmdline->RemoveParm("-height");
		cmdline->RemoveParm("+connect");
		cmdline->SetParm("-novid", nullptr);

		if (strstr(szNewCommandParams, "-game"))
			cmdline->RemoveParm("-game");

		if (strstr(szNewCommandParams, "+load"))
			cmdline->RemoveParm("+load");

		cmdline->AppendParm(szNewCommandParams, nullptr);

		g_pFileSystem->Unmount();
		Sys_UnloadModule(hModule);

		MH_Uninitialize();
	} while (bRestartEngine);

	registry->Shutdown();

#ifndef DEBUG
	::ReleaseMutex(hMutex);
	::CloseHandle(hMutex);
#endif

	::WSACleanup();

	return EXIT_SUCCESS;
}
