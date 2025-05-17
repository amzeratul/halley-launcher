#pragma once

#include <halley.hpp>

#include "launcher_settings.h"
using namespace Halley;

class LauncherSettings;

namespace Halley {
	class ILauncher;
}

class AddProject : public UIWidget {
public:
	AddProject(UIFactory& factory, LauncherSettings& settings, ILauncher& parent);

	void onMakeUI() override;

private:
	enum class Page {
		Main,
		OpenURL
	};

	UIFactory& factory;
	LauncherSettings& settings;
	ILauncher& parent;

	Page page = Page::Main;

	AliveFlag aliveFlag;

	void onNewProject();
	void onAddFromDisk();
	void onAddFromURL();
	void addNewProject(const ProjectLocation& project);

	void showError(const String& errorMessage);
	void showError(LocalisedString errorMessage);
	void setError(LocalisedString errorMessage);
	void close();
	void cancel();

	void setPage(Page page);
	bool canAddFromURL();
	void setURLPageEnabled(bool enabled);
};
