#include <WinSock2.h>
#include "steam_api.h"

#include "netadr.h"
#include "net.h"
#include "sv_main.h"

_NET_AdrToString ORIG_NET_AdrToString = NULL;
_NET_StringToAdr ORIG_NET_StringToAdr = NULL;

qboolean NET_StringToAdr(char* s, netadr_t* a)
{
	if (strncmp(s, "STEAM_", 6))
		return ORIG_NET_StringToAdr(s, a);

	EUniverse universe;
	uint32 authServer;
	uint32 accountNumber;
	sscanf(s, "STEAM_%u:%u:%u", &universe, &authServer, &accountNumber);

	// Only support individual for now.
	CSteamID steamID;
	steamID.Set((accountNumber << 1) | authServer, k_EUniversePublic, k_EAccountTypeIndividual);
	*(uint32*)a->ip = steamID.GetAccountID();
	a->port = htons(P2P_PORT);
	a->type = NA_IP;

	return TRUE;
}

char* NET_AdrToString(netadr_t a)
{
	static char buf[64];
	memset(buf, 0, sizeof(buf));

	if (a.type == NA_LOOPBACK)
	{
		_snprintf(buf, sizeof(buf), "loopback");
		return buf;
	}

	int port = ntohs(a.port);
	if (a.type == NA_IP)
	{
		if (port == P2P_PORT)
		{
			uint32 accountID = *(uint32*)a.ip;
			_snprintf(buf, sizeof(buf), "STEAM_%u:%u:%u", k_EUniversePublic, accountID & 1, accountID >> 1);
		}
		else
		{
			_snprintf(buf, sizeof(buf), "%i.%i.%i.%i:%i", a.ip[0], a.ip[1], a.ip[2], a.ip[3], port);
		}
		return buf;
	}

	_snprintf(buf, sizeof(buf), "%02x%02x%02x%02x:%02x%02x%02x%02x%02x%02x:%i",
		a.ipx[0], a.ipx[1], a.ipx[2], a.ipx[3], a.ipx[4],
		a.ipx[5], a.ipx[6], a.ipx[7], a.ipx[8], a.ipx[9],
		port);

	return buf;
}