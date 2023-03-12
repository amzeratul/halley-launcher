#pragma once

#include <halley.hpp>

class Settings;

namespace Halley {
	class ILauncher;

	class LaunchProject : public UIWidget {
    public:
    	LaunchProject(UIFactory& factory, Settings& settings, ILauncher& parent, Path path);

        void onMakeUI() override;
    	
    private:
    	UIFactory& factory;
        Settings& settings;
        ILauncher& parent;
        Path path;
    };
}
