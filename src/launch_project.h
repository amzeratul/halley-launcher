#pragma once

#include <halley.hpp>

#include "launcher_settings.h"

class LauncherSettings;

namespace Halley {
	class ILauncher;

	class LaunchProject : public UIWidget, ILoggerSink {
    public:
    	LaunchProject(UIFactory& factory, LauncherSettings& settings, ILauncher& parent, ProjectLocation project, bool safeMode);

        void onMakeUI() override;
        
	protected:
        void log(LoggerLevel level, std::string_view msg) override;

    private:
    	UIFactory& factory;
        LauncherSettings& settings;
        ILauncher& parent;
        ProjectLocation project;
        bool safeMode;

        Future<int> runningCommand;
        bool hasUI = false;

        HalleyVersion getBuiltVersion(const Path& path) const;
        void buildProject(bool clean);
        void launchProject();
    };
}
