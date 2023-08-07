#ifndef STEAMAPI_CONTEXT_H
#define STEAMAPI_CONTEXT_H

class P2PSteamAPIContext
{
public:
	bool Init();
	void Clear();

	ISteamClient* SteamClient() { return m_pSteamClient; }
	ISteamNetworkingMessages* SteamNetworkingMessages() { return m_pSteamNetworkingMessages; }

	STEAM_CALLBACK(P2PSteamAPIContext, OnSessionRequest, SteamNetworkingMessagesSessionRequest_t);

private:
	ISteamClient* m_pSteamClient;
	ISteamNetworkingMessages* m_pSteamNetworkingMessages;
};

extern P2PSteamAPIContext steam;

typedef bool (*_SteamAPI_Init)();
typedef void (*_SteamAPI_Shutdown)();

extern _SteamAPI_Init ORIG_SteamAPI_Init;
extern _SteamAPI_Shutdown ORIG_SteamAPI_Shutdown;

bool SteamInit();
void SteamShutdown();

void HookSteamAPI();

#endif // STEAMAPI_CONTEXT_H