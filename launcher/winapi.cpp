#include <WinSock2.h>
#include "steam_api.h"
#include "steamapi_context.h"

#include <MinHook.h>

#include "netadr.h"
#include "net.h"
#include "sv_main.h"

typedef int (WSAAPI* _SendTo)(SOCKET s, const char* buf, int len, int flags, const struct sockaddr* to, int tolen);
typedef int (WSAAPI* _RecvFrom)(SOCKET s, char* buf, int len, int flags, struct sockaddr* from, int* fromlen);

_SendTo ORIG_SendTo = NULL;
_RecvFrom ORIG_RecvFrom = NULL;

int WSAAPI SendTo(SOCKET s, const char* buf, int len, int flags, const struct sockaddr* to, int tolen)
{
	auto* addr = (const sockaddr_in*)to;
	if (addr->sin_port != htons(P2P_PORT))
	{
		return ORIG_SendTo(s, buf, len, flags, to, tolen);
	}

	// Only support individual for now.
	CSteamID steamID;
	steamID.Set(addr->sin_addr.S_un.S_addr, k_EUniversePublic, k_EAccountTypeIndividual);
	SteamNetworkingIdentity identity;
	identity.SetSteamID(steamID);
	auto result = steam.SteamNetworkingMessages()->SendMessageToUser(identity, buf, len, k_nSteamNetworkingSend_Unreliable | k_nSteamNetworkingSend_AutoRestartBrokenSession, 0);
	if (result != k_EResultOK)
	{
		WSASetLastError(WSAECONNREFUSED);
		return SOCKET_ERROR;
	}
	WSASetLastError(0);
	return len;
}

int WSAAPI RecvFrom(SOCKET s, char* buf, int len, int flags, struct sockaddr* from, int* fromlen)
{
	SteamNetworkingMessage_t* msg;
	if (steam.SteamNetworkingMessages()->ReceiveMessagesOnChannel(0, &msg, 1) != 1)
	{
		return ORIG_RecvFrom(s, buf, len, flags, from, fromlen);
	}

	int msgLen = (int)msg->GetSize();
	if (len < msgLen)
	{
		msg->Release();
		WSASetLastError(WSAEMSGSIZE);
		return SOCKET_ERROR;
	}

	auto* addr = (sockaddr_in*)from;
	addr->sin_family = AF_INET;
	addr->sin_addr.S_un.S_addr = msg->m_identityPeer.GetSteamID().GetAccountID();
	addr->sin_port = htons(P2P_PORT);
	memset(addr->sin_zero, 0, sizeof(addr->sin_zero));
	memcpy(buf, msg->GetData(), msgLen);
	msg->Release();
	WSASetLastError(0);
	return msgLen;
}

void HookWinAPI()
{
	HMODULE wsock32 = GetModuleHandle("wsock32");

	ORIG_SendTo = (_SendTo)GetProcAddress(wsock32, "sendto");
	ORIG_RecvFrom = (_RecvFrom)GetProcAddress(wsock32, "recvfrom");

	if (ORIG_SendTo && ORIG_RecvFrom)
	{
		void* pSendTo = (void*)ORIG_SendTo;
		void* pRecvFrom = (void*)ORIG_RecvFrom;

		MH_CreateHook(pSendTo, (void*)SendTo, (void**)&ORIG_SendTo);
		MH_CreateHook(pRecvFrom, (void*)RecvFrom, (void**)&ORIG_RecvFrom);
	}
	else
		printf("Couldn't hook \"sendto\" and \"recvfrom\".\n");
}