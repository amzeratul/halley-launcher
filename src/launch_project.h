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
        void update(Time t, bool moved) override;
        
	protected:
        void log(LoggerLevel level, std::string_view msg) override;

    private:
    	UIFactory& factory;
        LauncherSettings& settings;
        ILauncher& parent;
        ProjectLocation projectLocation;
        bool safeMode;
        AliveFlag aliveFlag;

        Future<int> runningCommand;
        bool hasUI = false;

        std::mutex progressMutex;
		std::optional<std::pair<uint64_t, uint64_t>> pendingProgress;

        void loadUIIfNeeded();
        void tryLaunching();
        void buildProject(bool clean);
        void downloadEditor(HalleyVersion version);
        void installEditor(Bytes data);
        bool doInstallEditor(Bytes data, const Path& path);
        void launchProject();

        void setProgress(uint64_t progress, uint64_t total);

        void checkForProjectUpdates();
    };
}
