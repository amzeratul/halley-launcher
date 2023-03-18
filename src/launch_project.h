#pragma once

#include <halley.hpp>

class Settings;

namespace Halley {
	class ILauncher;

	class LaunchProject : public UIWidget, ILoggerSink {
    public:
    	LaunchProject(UIFactory& factory, Settings& settings, ILauncher& parent, Path path);

        void onMakeUI() override;
        
	protected:
        void log(LoggerLevel level, std::string_view msg) override;

    private:
    	UIFactory& factory;
        Settings& settings;
        ILauncher& parent;
        Path path;

        Future<int> runningCommand;
        bool hasUI = false;

        HalleyVersion getBuiltVersion(const Path& path) const;
        void buildProject(bool clean);
        void launchProject();
    };
}
