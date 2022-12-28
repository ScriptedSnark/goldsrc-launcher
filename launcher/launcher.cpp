#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <direct.h>
#include <io.h>
#include <process.h>

#include <WinSock2.h>

#include "engine_launcher_api.h"
#include "FilePaths.h"
#include "FileSystem.h"
#include "ICommandLine.h"
#include "interface.h"
#include "IRegistry.h"

#include "MinHook.h"
#include "goldsrc_hook.h"

#define HARDWARE_ENGINE "hw.dll"
#define SOFTWARE_ENGINE "sw.dll"

bool TextMode = false;

char com_gamedir[MAX_PATH] = {};

IFileSystem* g_pFileSystem = nullptr;

bool Sys_GetExecutableName(char* out, int len)
{
	if (!::GetModuleFileName((HINSTANCE)GetModuleHandle(NULL), out, len))
		return false;

	return true;
}

BOOL WINAPI MyHandlerRoutine(DWORD dwCtrlType)
{
	TerminateProcess(GetCurrentProcess(), 2);
	return TRUE;
}

void InitTextMode()
{
	AllocConsole();

	SetConsoleCtrlHandler(MyHandlerRoutine, TRUE);

	freopen("CONIN$", "rb", stdin);	  // reopen stdin handle as console window input
	freopen("CONOUT$", "wb", stdout); // reopen stout handle as console window output
	freopen("CONOUT$", "wb", stderr); // reopen stderr handle as console window output
}

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

bool OnVideoModeFailed(void)
{
	registry->WriteInt("ScreenBPP", 16);
	registry->WriteInt("ScreenHeight", 640);
	registry->WriteInt("ScreenWidth", 480);

	registry->WriteString("EngineDLL", HARDWARE_ENGINE);

	return MessageBox(NULL, "The specified video mode is not supported.", "Video mode change failure", MB_OKCANCEL | MB_ICONERROR | MB_ICONQUESTION) == IDOK;
}

int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	//If we aren't allowed to continue launching, exit now.
	if (LR_VerifySteamStatus(lpCmdLine, filepath::FILESYSTEM_STDIO, filepath::FILESYSTEM_STDIO))
		return EXIT_SUCCESS;

#ifndef DEBUG
	HANDLE hMutex = CreateMutexA(nullptr, FALSE, "ValveHalfLifeLauncherMutex");

	if (NULL != hMutex)
	{
		GetLastError();
	}

	DWORD dwMutexResult = WaitForSingleObject(hMutex, 0);

	if (dwMutexResult != WAIT_OBJECT_0 && dwMutexResult != WAIT_ABANDONED)
	{
		MessageBoxA(NULL, "Could not launch game.\nOnly one instance of this game can be run at a time.", "Fatal Error", MB_OK | MB_ICONERROR);
		return EXIT_SUCCESS;
	}
#endif

	MH_Initialize();

	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 0), &WSAData);

	registry->Init();

	cmdline->CreateCmdLine(GetCommandLineA());

	char Filename[256];

	Sys_GetExecutableName(Filename, sizeof(Filename));

	//If this isn't hl.exe, force the game to be whichever game this exe is for.
	{
		char* pszLastSlash = strrchr(Filename, '\\') + 1;

		//E.g. cstrike.exe -> -game cstrike.
		if (_stricmp("hl.exe", pszLastSlash) && !cmdline->CheckParm("-game", nullptr))
		{
			//This assumes that the program extension is ".exe" or another 4 character long extension.
			pszLastSlash[strlen(pszLastSlash) - 4] = '\0';
			cmdline->SetParm("-game", pszLastSlash);
		}
	}

	//Set the game name.
	{
		//TODO: this is wrong. pszGame will point to the rest of the command line after and including -game.
		//It should copy the second parameter's result value. - Solokiller
		const char* pszGame = cmdline->CheckParm("-game", nullptr);

		//Default to Half-Life.
		if (!pszGame)
			pszGame = "valve";

		strncpy(com_gamedir, pszGame, sizeof(com_gamedir));
		com_gamedir[sizeof(com_gamedir) - 1] = '\0';
	}

	if (cmdline->CheckParm("-textmode", nullptr))
	{
		TextMode = true;
		InitTextMode();
		// TODO: hide Half-Life window - ScriptedSnark
	}

	// Make low priority?
	if (cmdline->CheckParm("-low", nullptr))
	{
		SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);
	}
	else if (cmdline->CheckParm("-high", nullptr))
	{
		SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	}

#ifndef DEBUG
	_unlink("mssv29.asi");
	_unlink("mssv12.asi");
	_unlink("mp3dec.asi");
	_unlink("opengl32.dll");
#endif

	//If the game crashed during video mode initialization, reset video mode to default.
	if (registry->ReadInt("CrashInitializingVideoMode", 0))
	{
		registry->WriteInt("CrashInitializingVideoMode", 0);

		const char* pszEngineDLL = registry->ReadString("EngineDLL", "hw.dll");

		if (!_stricmp(pszEngineDLL, "hw.dll"))
		{
			const char* pszCaption = "Video mode change failure";
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
				//TODO: Shouldn't this be sw.dll? - Solokiller
				registry->WriteString("EngineDLL", "hw.dll");

				pszMessage =
					"The game has detected that the previous attempt to start in openGL video mode failed.\n"
					"The game will now run in software mode.";
			}

			//Ask the user if they want to continue.
			if (MessageBoxA(NULL, pszMessage, pszCaption, MB_OKCANCEL | MB_ICONERROR | MB_ICONQUESTION) != IDOK)
				return EXIT_SUCCESS;

			registry->WriteInt("ScreenBPP", 16);
			registry->WriteInt("ScreenHeight", 640);
			registry->WriteInt("ScreenWidth", 480);
		}
	}

	static char szNewCommandParams[2048];

	bool bRestartEngine = false;

	do
	{
		//Load and mount the filesystem.
		CSysModule* hModule = LoadFilesystemModule(Filename, cmdline->CheckParm("-game", nullptr) != nullptr);

		if (!hModule)
			break;

		{
			CreateInterfaceFn factoryFn = Sys_GetFactory(hModule);

			g_pFileSystem = static_cast<IFileSystem*>(factoryFn(FILESYSTEM_INTERFACE_VERSION, nullptr));
		}

		g_pFileSystem->Mount();

		g_pFileSystem->AddSearchPath(UTIL_GetBaseDir(), "ROOT");

		bRestartEngine = false;
		EngineRunResult runResult = ENGRUN_QUITTING;

		szNewCommandParams[0] = '\0';

		const char* pszLibFileName;
		SetEngineDLL(&pszLibFileName);

		CSysModule* hLibModule = Sys_LoadModule(pszLibFileName);

		HookEngine(); // it must be here!!!

		if (hLibModule)
		{
			CreateInterfaceFn factoryFn = Sys_GetFactory(hLibModule);

			if (factoryFn)
			{
				IEngineAPI* pEngine = static_cast<IEngineAPI*>(factoryFn(ENGINE_LAUNCHER_INTERFACE_VERSION, nullptr));

				if (pEngine)
				{
					runResult = static_cast<EngineRunResult>(
						pEngine->Run(
							hInstance,
							UTIL_GetBaseDir(),
							cmdline->GetCmdLine(),
							szNewCommandParams,
							Sys_GetFactoryThis(),
							Sys_GetFactory(hModule)));
				}
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
		case ENGRUN_QUITTING:
			bRestartEngine = false;
			break;
		case ENGRUN_CHANGED_VIDEOMODE:
			bRestartEngine = true;
			break;
		case ENGRUN_UNSUPPORTED_VIDEOMODE:
			bRestartEngine = OnVideoModeFailed();
			break;
		default:
			break;
		}

		//If we're restarting, remove any parameters that could affect video mode changes.
		//Also remove parameters that trigger events automatically, such as connecting to a server.
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

		//User changed game.
		if (strstr(szNewCommandParams, "-game"))
		{
			cmdline->RemoveParm("-game");
		}

		//Remove saved game load command if new command is present.
		if (strstr(szNewCommandParams, "+load"))
		{
			cmdline->RemoveParm("+load");
		}

		//Append new command line to process properly.
		cmdline->AppendParm(szNewCommandParams, nullptr);

		g_pFileSystem->Unmount();
		Sys_UnloadModule(hModule);
	} while (bRestartEngine);

	MH_Uninitialize();

	registry->Shutdown();

#ifndef DEBUG
	ReleaseMutex(hMutex);
	CloseHandle(hMutex);
#endif

	WSACleanup();

	return EXIT_SUCCESS;
}
