#include "add_project.h"

#include "choose_project.h"
#include "launcher_project_properties.h"
#include "launcher_settings.h"
#include "launcher_stage.h"

AddProject::AddProject(UIFactory& factory, LauncherSettings& settings, ILauncher& parent)
	: UIWidget("add_project", {}, UISizer())
	, factory(factory)
	, settings(settings)
	, parent(parent)
{
	factory.loadUI(*this, "launcher/add_project");
}

void AddProject::onMakeUI()
{
	setHandle(UIEventType::ButtonClicked, "create", [=] (const UIEvent& event)
	{
		onNewProject();
	});

	setHandle(UIEventType::ButtonClicked, "addLocal", [=] (const UIEvent& event)
	{
		onAddFromDisk();
	});

	setHandle(UIEventType::ButtonClicked, "addURLStart", [=] (const UIEvent& event)
	{
		setPage(Page::OpenURL);
	});

	setHandle(UIEventType::ButtonClicked, "addFromURL", [=] (const UIEvent& event)
	{
		onAddFromURL();
	});

	setHandle(UIEventType::ButtonClicked, "cancel", [=] (const UIEvent& event)
	{
		if (page == Page::Main) {
			close();
		} else {
			setPage(Page::Main);
		}
	});

	setPage(Page::Main);
}

void AddProject::onNewProject()
{
	// TODO
}

void AddProject::onAddFromDisk()
{
	FileChooserParameters parameters;
	parameters.folderOnly = true;
	OS::get().openFileChooser(parameters).then(Executors::getMainUpdateThread(), [=] (std::optional<Path> path)
	{
		if (path) {
			addNewProject(ProjectLocation(path.value().getNativeString(false)));
		}
	});
}

void AddProject::onAddFromURL()
{
	// TODO
}

void AddProject::addNewProject(const ProjectLocation& project)
{
	if (const auto properties = LauncherProjectProperties::getProjectProperties(project, &factory.getResources(), parent.getHalleyAPI().video)) {
		if (settings.addProject(project.path)) {
			close();
		} else {
			showError(LocalisedString::fromHardcodedString("Error: Project already exists."));
		}
	} else {
		showError(LocalisedString::fromHardcodedString("Error: Unable to add project."));
	}
}

void AddProject::showError(LocalisedString errorMessage)
{
	setError(std::move(errorMessage));
	setPage(Page::Main);
}

void AddProject::setError(LocalisedString msg)
{
	auto label = getWidgetAs<UILabel>("statusMessage");
	label->setActive(!msg.getString().isEmpty());
	label->setText(std::move(msg));
}

void AddProject::close()
{
	parent.switchTo(std::make_shared<ChooseProject>(factory, settings, parent));
}

void AddProject::setPage(Page page)
{
	this->page = page;

	getWidget("mainPage")->setActive(page == Page::Main);
	getWidget("addURLPage")->setActive(page == Page::OpenURL);
}
