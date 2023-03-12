#pragma once

#include <halley.hpp>

class Settings;

namespace Halley {
	class ILauncher;

	class ChooseProject : public UIWidget {
    public:
    	ChooseProject(UIFactory& factory, VideoAPI& videoAPI, Settings& settings, ILauncher& parent);

        void onMakeUI() override;
    	
    private:
        struct ProjectProperties {
            Path path;
            String name;
            Sprite icon;
            int halleyVerMajor = 0;
            int halleyVerMinor = 0;
            int halleyVerRevision = 0;
        };

    	UIFactory& factory;
        VideoAPI& videoAPI;
        Settings& settings;
        ILauncher& parent;
        
        void onNew();
        void onAdd();
        void onOpen(const Path& path);

    	void loadPaths();
    	void addNewPath(const Path& path);
        void addPathToList(const ProjectProperties& properties);

        std::optional<ProjectProperties> getProjectProperties(const Path& path) const;
    };
}
