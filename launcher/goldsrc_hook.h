#ifdef GOLDSRC_HOOK_H_RECURSE_GUARD
#error Recursive header files inclusion detected in goldsrc_hook.h
#else //GOLDSRC_HOOK_H_RECURSE_GUARD

#define GOLDSRC_HOOK_H_RECURSE_GUARD

#ifndef GOLDSRC_HOOK_H_GUARD
#define GOLDSRC_HOOK_H_GUARD
#pragma once

void HookEngine();

#endif //GOLDSRC_HOOK_H_GUARD

#undef GOLDSRC_HOOK_H_RECURSE_GUARD
#endif //GOLDSRC_HOOK_H_RECURSE_GUARD