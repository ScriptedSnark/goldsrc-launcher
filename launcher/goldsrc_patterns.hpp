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
}
}

#endif //GOLDSRC_PATTERNS_HPP_GUARD

#undef GOLDSRC_PATTERNS_HPP_RECURSE_GUARD
#endif //GOLDSRC_PATTERNS_HPP_RECURSE_GUARD