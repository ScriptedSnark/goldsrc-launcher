#ifndef PUBLIC_ENGINE_LAUNCHER_API_H
#define PUBLIC_ENGINE_LAUNCHER_API_H

#include "interface.h"

/**
*	Interface exposed by the engine to the launcher.
*/
class IEngineAPI : public IBaseInterface
{
public:
	virtual int Run( void* instance, char* basedir, char* cmdline, char* postRestartCmdLineArgs, CreateInterfaceFn launcherFactory, CreateInterfaceFn filesystemFactory ) = 0;
};

#define ENGINE_LAUNCHER_INTERFACE_VERSION "VENGINE_LAUNCHER_API_VERSION002"

#endif //PUBLIC_ENGINE_LAUNCHER_API_H
