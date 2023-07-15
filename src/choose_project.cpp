#include "choose_project.h"

#include "launcher_stage.h"
#include "launch_project.h"
#include "settings.h"
using namespace Halley;

ChooseProject::ChooseProject(UIFactory& factory, VideoAPI& videoAPI, Settings& settings, ILauncher& parent)
	: UIWidget("choose_project", {}, UISizer())
	, factory(factory)
	, videoAPI(videoAPI)
	, settings(settings)
	, parent(parent)
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
	auto logo = getWidgetAs<UIImage>("logo");
	logo->setActive(true);
	logo->setSprite(std::move(halleyLogo));

	onProjectSelected({});
	
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
		Concurrent::execute(Executors::getMainUpdateThread(), [=]() {
			onOpen(getWidgetAs<UIList>("projects")->getSelectedOptionId());
		});
	});
	
	setHandle(UIEventType::ButtonClicked, "openSafe", [=] (const UIEvent& event)
	{
		Concurrent::execute(Executors::getMainUpdateThread(), [=]() {
			onOpen(getWidgetAs<UIList>("projects")->getSelectedOptionId(), true);
		});
	});

	setHandle(UIEventType::ButtonClicked, "updateLauncher", [=] (const UIEvent& event)
	{
		Concurrent::execute(Executors::getMainUpdateThread(), [=]() {
			onUpdateLauncher();
		});
	});

	setHandle(UIEventType::ListSelectionChanged, "projects", [=](const UIEvent& event)
	{
		Concurrent::execute(Executors::getMainUpdateThread(), [=]() {
			onProjectSelected(event.getStringData());
		});
	});

	setHandle(UIEventType::ListAccept, "projects", [=] (const UIEvent& event)
	{
		Concurrent::execute(Executors::getMainUpdateThread(), [=]() {
			onOpen(event.getStringData());
		});
	});
}

void ChooseProject::update(Time t, bool moved)
{
	const auto newVersionInfo = parent.getNewVersionInfo();
	getWidget("updateLauncher")->setActive(newVersionInfo && newVersionInfo->isNewVersion());
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

void ChooseProject::onOpen(const Path& path, bool safeMode)
{
	if (!path.isEmpty()) {
		settings.bumpProject(path.getString());
		parent.switchTo(std::make_shared<LaunchProject>(factory, settings, parent, path, safeMode));
	}
}

void ChooseProject::onProjectSelected(const Path& path)
{
	bool enabled = false;
	bool safeEnabled = false;
	if (!path.isEmpty()) {
		if (const auto properties = ProjectProperties::getProjectProperties(Path(path), &factory.getResources(), &videoAPI)) {
			enabled = true;
			safeEnabled = properties->halleyVersion >= HalleyVersion{3, 3, 79};
		}
	}
	getWidget("open")->setEnabled(enabled);
	getWidget("openSafe")->setEnabled(safeEnabled);
}

void ChooseProject::onUpdateLauncher()
{
	parent.switchTo("update");
}

void ChooseProject::loadPaths()
{
	Vector<String> toRemove;
	for (const auto& path: settings.getProjects()) {
		if (auto properties = ProjectProperties::getProjectProperties(Path(path), &factory.getResources(), &videoAPI)) {
			addPathToList(*properties);
		} else {
			toRemove.push_back(path);
		}
	}
	for (const auto& path: toRemove) {
		settings.removeProject(path);
	}
}

void ChooseProject::addNewPath(const Path& path)
{
	if (const auto properties = ProjectProperties::getProjectProperties(path, &factory.getResources(), &videoAPI)) {
		const bool added = settings.addProject(path.toString());
		if (added) {
			addPathToList(*properties);
		}
	} else {
		// TODO: report error
	}
}

void ChooseProject::addPathToList(const ProjectProperties& properties)
{
	const auto list = getWidgetAs<UIList>("projects");

	const auto id = properties.path.getString();

	auto entry = factory.makeUI("launcher/project_entry");
	entry->getWidgetAs<UILabel>("project_name")->setText(LocalisedString::fromUserString(properties.name));
	entry->getWidgetAs<UILabel>("project_path")->setText(LocalisedString::fromUserString(properties.path.getNativeString(false)));
	entry->getWidgetAs<UILabel>("halley_version")->setText(LocalisedString::fromUserString("Halley v" + properties.halleyVersion.toString()));

	if (properties.icon.hasMaterial()) {
		entry->getWidgetAs<UIImage>("project_icon")->setSprite(properties.icon);
	}

	entry->setHandle(UIEventType::ButtonClicked, "delete", [=] (const UIEvent& event)
	{
		Concurrent::execute(Executors::getMainUpdateThread(), [=]() {
			settings.removeProject(id);
			list->removeItem(id);
		});
	});

	list->addItem(id, std::move(entry), 1);
}
