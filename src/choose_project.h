#pragma once

#include <halley.hpp>
#include "launcher_project_properties.h"

class LauncherSettings;

namespace Halley {
	class ILauncher;

	class ChooseProject : public UIWidget {
    public:
    	ChooseProject(UIFactory& factory, VideoAPI& videoAPI, LauncherSettings& settings, ILauncher& parent);

        void onMakeUI() override;
        void update(Time t, bool moved) override;
    	
    private:
    	UIFactory& factory;
        VideoAPI& videoAPI;
        LauncherSettings& settings;
        ILauncher& parent;
        
        void onNew();
        void onAdd();
        void onOpen(const String& path, bool safeMode = false);
        void onProjectSelected(const String& path);
        void onUpdateLauncher();

    	void loadPaths();
    	void addNewProject(const ProjectLocation& project);
        void addPathToList(const LauncherProjectProperties& properties);
    };
}
