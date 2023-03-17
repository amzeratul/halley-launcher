#pragma once

#include <halley.hpp>
#include "project_properties.h"

class Settings;

namespace Halley {
	class ILauncher;

	class ChooseProject : public UIWidget {
    public:
    	ChooseProject(UIFactory& factory, VideoAPI& videoAPI, Settings& settings, ILauncher& parent);

        void onMakeUI() override;
        void update(Time t, bool moved) override;
    	
    private:
    	UIFactory& factory;
        VideoAPI& videoAPI;
        Settings& settings;
        ILauncher& parent;
        
        void onNew();
        void onAdd();
        void onOpen(const Path& path);
        void onUpdateLauncher();

    	void loadPaths();
    	void addNewPath(const Path& path);
        void addPathToList(const ProjectProperties& properties);
    };
}
