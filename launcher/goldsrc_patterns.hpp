#ifdef GOLDSRC_PATTERNS_HPP_RECURSE_GUARD
#error Recursive header files inclusion detected in goldsrc_patterns.hpp
#else //GOLDSRC_PATTERNS_HPP_RECURSE_GUARD

#define GOLDSRC_PATTERNS_HPP_RECURSE_GUARD

#ifndef GOLDSRC_PATTERNS_HPP_GUARD
#define GOLDSRC_PATTERNS_HPP_GUARD
#pragma once

#include "patterns.hpp"
#include "MemUtils.h"

namespace patterns
{
namespace engine
{
PATTERNS(AdjustWindowForMode,
	"GoldSrc-8684",
	"53 56 8B F1 57 8B 06 FF 50 ??");

PATTERNS(NET_AdrToString,
	"HL-SteamPipe-8684",
	"55 8B EC 6A 40 6A 00 68 ?? ?? ?? ?? E8 2F 22 FC FF 8B 45 08 83 C4 0C 83 F8 01 75 1B");

PATTERNS(NET_StringToAdr,
	"HL-SteamPipe-8684",
	"55 8B EC 83 EC 10 56 8B 75 ?? 68 ?? ?? ?? ??");

PATTERNS(SV_CheckIPRestrictions,
	"HL-SteamPipe-8684",
	"55 8B EC D9 05 ?? ?? ?? ?? D8 1D ?? ?? ?? ?? 53 56 57 DF E0 F6 C4 44 8B 45");
}
}

#endif //GOLDSRC_PATTERNS_HPP_GUARD

#undef GOLDSRC_PATTERNS_HPP_RECURSE_GUARD
#endif //GOLDSRC_PATTERNS_HPP_RECURSE_GUARD