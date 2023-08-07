#include <WinSock2.h>

#include "steam_api.h"
#include "interface.h"
#include "steamapi_context.h"

#include "MinHook.h"

P2PSteamAPIContext steam;

bool P2PSteamAPIContext::Init()
{
	auto steamClientModule = Sys_LoadModule("steamclient.dll");
	auto steamClientFactory = Sys_GetFactory(steamClientModule);

	m_pSteamClient = (ISteamClient*)steamClientFactory(STEAMCLIENT_INTERFACE_VERSION, nullptr);

	if (m_pSteamClient == nullptr)
		return false;

	HSteamUser hSteamUser = SteamAPI_GetHSteamUser();
	HSteamPipe hSteamPipe = SteamAPI_GetHSteamPipe();

	m_pSteamNetworkingMessages = (ISteamNetworkingMessages*)SteamClient()->GetISteamGenericInterface(hSteamUser, hSteamPipe, STEAMNETWORKINGMESSAGES_INTERFACE_VERSION);

	if (m_pSteamNetworkingMessages == nullptr)
		return false;

	return true;
}

void P2PSteamAPIContext::Clear()
{
	memset(this, 0, sizeof(*this));
}

void P2PSteamAPIContext::OnSessionRequest(SteamNetworkingMessagesSessionRequest_t* param)
{
	steam.SteamNetworkingMessages()->AcceptSessionWithUser(param->m_identityRemote);
}

bool SteamInit()
{
	if (!ORIG_SteamAPI_Init())
		return false;

	printf("Initializing Steam API...\n");

	return steam.Init();
}

void SteamShutdown()
{
	steam.Clear();

	ORIG_SteamAPI_Shutdown();
}

void HookSteamAPI()
{
	HMODULE steam_api = GetModuleHandle("steam_api");
	ORIG_SteamAPI_Init = (_SteamAPI_Init)GetProcAddress(steam_api, "SteamAPI_Init");
	ORIG_SteamAPI_Shutdown = (_SteamAPI_Shutdown)GetProcAddress(steam_api, "SteamAPI_Shutdown");

	if (ORIG_SteamAPI_Init)
		printf("[steam_api] Got SteamAPI_Init!\n");
	else
		printf("[steam_api] Can't get SteamAPI_Init!\n");

	if (ORIG_SteamAPI_Shutdown)
		printf("[steam_api] Got SteamAPI_Shutdown!\n");
	else
		printf("[steam_api] Can't get SteamAPI_Shutdown!\n");

	if (ORIG_SteamAPI_Init)
	{
		void* pSteamAPI_Init = (void*)ORIG_SteamAPI_Init;
		MH_CreateHook(pSteamAPI_Init, (void*)SteamInit, (void**)&ORIG_SteamAPI_Init);
		MH_EnableHook(pSteamAPI_Init);
	}

	if (ORIG_SteamAPI_Init)
	{
		void* pSteamAPI_Shutdown = (void*)ORIG_SteamAPI_Shutdown;
		MH_CreateHook(pSteamAPI_Shutdown, (void*)SteamShutdown, (void**)&ORIG_SteamAPI_Shutdown);
		MH_EnableHook(pSteamAPI_Shutdown);
	}
}