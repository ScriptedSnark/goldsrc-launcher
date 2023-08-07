#include <WinSock2.h>
#include "steam_api.h"

#include "netadr.h"
#include "net.h"
#include "sv_main.h"

_SV_CheckIPRestrictions ORIG_SV_CheckIPRestrictions = NULL;

int SV_CheckIPRestrictions(netadr_t* adr, int nAuthProtocol)
{
	if (adr->type == NA_IP && adr->port == htons(P2P_PORT))
		return 1;

	return ORIG_SV_CheckIPRestrictions(adr, nAuthProtocol);
}