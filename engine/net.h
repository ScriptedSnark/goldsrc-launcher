#ifdef NET_H_RECURSE_GUARD
#error Recursive header files inclusion detected in net.h
#else //NET_H_RECURSE_GUARD

#define NET_H_RECURSE_GUARD

#ifndef NET_H_GUARD
#define NET_H_GUARD
#pragma once

typedef int qboolean; // ...

typedef char* (*_NET_AdrToString)(netadr_t a);
typedef qboolean (*_NET_StringToAdr)(char* s, netadr_t* a);
typedef void (*_SCR_DrawFPS)();

extern _NET_AdrToString ORIG_NET_AdrToString;
extern _NET_StringToAdr ORIG_NET_StringToAdr;

char* NET_AdrToString(netadr_t a);
qboolean NET_StringToAdr(char* s, netadr_t* a);

#endif //NET_H_GUARD

#undef NET_H_RECURSE_GUARD
#endif //NET_H_RECURSE_GUARD