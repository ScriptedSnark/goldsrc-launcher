#ifdef PLUGINS_LOADER_HPP_RECURSE_GUARD
#error Recursive header files inclusion detected in PLUGINS_loader.hpp
#else //PLUGINS_LOADER_HPP_RECURSE_GUARD

#define PLUGINS_LOADER_HPP_RECURSE_GUARD

#ifndef PLUGINS_LOADER_HPP_GUARD
#define PLUGINS_LOADER_HPP_GUARD
#pragma once

class CPluginsLoader
{
public:
	void InitializePlugin(HMODULE PLUGINS);
	void LoadPlugins();
};

#endif //PLUGINS_LOADER_HPP_GUARD

#undef PLUGINS_LOADER_HPP_RECURSE_GUARD
#endif //PLUGINS_LOADER_HPP_RECURSE_GUARD