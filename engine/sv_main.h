#ifdef SV_MAIN_H_RECURSE_GUARD
#error Recursive header files inclusion detected in sv_main.h
#else // SV_MAIN_H_RECURSE_GUARD

#define SV_MAIN_H_RECURSE_GUARD

#ifndef SV_MAIN_H_GUARD
#define SV_MAIN_H_GUARD
#pragma once

static constexpr uint16 P2P_PORT = 1;

typedef int (*_SV_CheckIPRestrictions)(netadr_t* adr, int nAuthProtocol);
extern _SV_CheckIPRestrictions ORIG_SV_CheckIPRestrictions;

int SV_CheckIPRestrictions(netadr_t* adr, int nAuthProtocol);

#endif // SV_MAIN_H_GUARD

#undef SV_MAIN_H_RECURSE_GUARD
#endif // SV_MAIN_H_RECURSE_GUARD