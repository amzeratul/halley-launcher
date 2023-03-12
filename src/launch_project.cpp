#include "launch_project.h"
using namespace Halley;

LaunchProject::LaunchProject(UIFactory& factory, Settings& settings, ILauncher& parent, Path path)
	: factory(factory)
	, settings(settings)
	, parent(parent)
	, path(std::move(path))
{
}

void LaunchProject::onMakeUI()
{
	
}
