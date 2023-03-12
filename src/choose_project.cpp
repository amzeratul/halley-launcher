#include "choose_project.h"
using namespace Halley;

ChooseProject::ChooseProject(UIFactory& factory)
	: UIWidget("choose_project", {}, UISizer())
	, factory(factory)
{
	factory.loadUI(*this, "launcher/load_project");

	loadPaths();
}

void ChooseProject::onMakeUI()
{
	const auto col = factory.getColourScheme()->getColour("logo");
	auto halleyLogo = Sprite()
		.setImage(factory.getResources(), "halley/halley_logo_dist.png", "Halley/DistanceFieldSprite")
		.setScale(1)
		.setColour(col);
	halleyLogo.getMutableMaterial()
		.set("u_smoothness", 16.0f)
		.set("u_outline", 0.0f)
		.set("u_outlineColour", col);
	getWidgetAs<UIImage>("logo")->setSprite(std::move(halleyLogo));
	
	setHandle(UIEventType::ButtonClicked, "new", [=] (const UIEvent& event)
	{
		onNew();
	});

	setHandle(UIEventType::ButtonClicked, "add", [=] (const UIEvent& event)
	{
		onAdd();
	});
	
	setHandle(UIEventType::ButtonClicked, "open", [=] (const UIEvent& event)
	{
		onOpen();
	});
}

void ChooseProject::onNew()
{
	// TODO
}

void ChooseProject::onAdd()
{
	FileChooserParameters parameters;
	parameters.folderOnly = true;
	OS::get().openFileChooser(parameters).then(Executors::getMainUpdateThread(), [=] (std::optional<Path> path)
	{
		if (path) {
			addNewPath(path.value());
		}
	});
}

void ChooseProject::onOpen()
{
	// TODO
}

void ChooseProject::loadPaths()
{
	refresh();
}

void ChooseProject::savePaths()
{
	
}

void ChooseProject::addNewPath(Path path)
{
	addPath(path);
	savePaths();
	refresh();
}

void ChooseProject::addPath(Path path)
{
	paths.emplace_back(std::move(path));
}

void ChooseProject::refresh()
{
	
}
